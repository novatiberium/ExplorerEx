//+-------------------------------------------------------------------------
//
//  CabinetEx - Windows NT Explorer
//  Copyright (C) Microsoft
//
//  File:       shundoc.c
//
//  History:    Jan-27-25   kfh83  Created
//  
//--------------------------------------------------------------------------

#include "..\Headers\shundoc.h"
#include "ShlObj.h"
#include "..\Headers\ids.h"
#include "..\Headers\port32.h"
#include "Shlwapi.h"

//
//  Constants
//

/* GLOBAL Variables used for DDESPY */

UINT        idInst = 0;
HINSTANCE   hInst;
HICON       hIcon;
HWND        hWndString = NULL;
HWND        hwndSpy = NULL;
HANDLE      fhOutput = NULL;
OFSTRUCT    ofsOpen;
TCHAR        OpenName[10];
TCHAR        TBuf[512];
TCHAR        TBuf2[512];
TCHAR        szNULL[] = TEXT("");
LPTSTR        apszResources[125];
PFNCALLBACK pfnDdeCallback = NULL;
HWND        hwndTrack[4] = { 0 };
LPTSTR        TrackTitle[4];
BOOL        fBlockMsg[WM_DDE_LAST - WM_DDE_FIRST + 1] = { 0 };
BOOL        fBlockCb[15] = { 0 };
LPTSTR        TrackHeading[4];

const TCHAR c_szColonSlash[] = TEXT(":\\");

const IDLREGITEM c_idlNet;
const IDLREGITEM c_idlDrives;


BOOL(STDMETHODCALLTYPE* SHFindComputer)(LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidlSaveFile);
BOOL(STDMETHODCALLTYPE* RegisterShellHook)(HWND hwnd, BOOL fInstall);
BOOL(STDMETHODCALLTYPE* FileIconInit)(BOOL fRestoreCache);
INT(STDMETHODCALLTYPE* RunFileDlg)(HWND hwndParent, HICON hIcon, LPCTSTR pszWorkingDir, LPCTSTR pszTitle, LPCTSTR pszPrompt, DWORD dwFlags);
HRESULT(STDMETHODCALLTYPE* ExitWindowsDialog)(HWND hwndParent);
BOOL(WINAPI* DrawCaptionTempW)(HWND, HDC, LPRECT, HFONT, HICON, LPWSTR, UINT);
BOOL(WINAPI* ILGetDisplayName)(LPCITEMIDLIST pidl, LPTSTR pszPath);

BOOL(WINAPI* SetShellWindow)(HWND hwnd);

//
//	Classes
//

// 
//	Function definitions
//

HINSTANCE g_hinst = NULL;


BOOL _SHChangeNotifyAddEventToClientQueuesUnimpl(LONG lEvent, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra)
{
    return TRUE;
}

static FSNotifyPerProc sp_fsn =
{
        NULL,
        0,
        NULL,
        0,
        0,
        0,
        NULL,
        NULL,
        0,
        FALSE
};


void SHChangeNotifyReceive(LONG lEvent, UINT uFlags, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra)
{
    BOOL bOnlyUpdateDirs;

    /// now do the actual generating of the event
    if (lEvent & (SHCNE_NETSHARE | SHCNE_NETUNSHARE))
    {
        // Update the cache.

        IsILShared(pidl, TRUE);
    }
#ifdef FSNDEBUG
    DebugMsg(DM_TRACE, TEXT("SHChangeNotifyGenreateEvent 0x%X"), lEvent);
    DebugDumpPidl(TEXT("SHChangeNotifyGenerateEvent"), pidl);
#endif

    if (lEvent)
        bOnlyUpdateDirs = _SHChangeNotifyAddEventToClientQueuesUnimpl(lEvent, pidl, pidlExtra);

    // remove any shell generated events for the file system
    if ((lEvent & SHCNE_DISKEVENTS) &&
        !(lEvent & (SHCNE_INTERRUPT | SHCNE_UPDATEDIR | SHCNE_UPDATEITEM))) {
        if (!bOnlyUpdateDirs)
        {
            // No use waiting for events to come through if everybody is just
            // doing an updatedir anyway.  Note that we will still clear out
            // as amny int events as we can so that we do not fill up that
            // queue
            //_WaitFSThreadProcessEvents();
        }

        FSNRemoveInterruptEvent(pidl);
        if (lEvent & (SHCNE_RENAMEFOLDER | SHCNE_RENAMEITEM)) {
            FSNRemoveInterruptEvent(pidlExtra);
        }

    }

    //
    // note make sure the internal events go first.
    //
    // unless the nonotifyinteranls flag is set meaning that this was created
    // by ultroot's desktop relayer (CDesktop_FSEvent), in which case don't bother
    // checking everything again
    //if (lEvent && (!(uFlags & SHCNF_NONOTIFYINTERNALS)))
        //NotifyShellInternals(lEvent, pidl, pidlExtra);

    //
    // then the registered events
    //
    if (uFlags & (SHCNF_FLUSH)) {
        if (uFlags & (SHCNF_FLUSHNOWAIT)) {
            sp_fsn.fFlushNow = TRUE;
            //WakeThread(sp_fsn.idtRunning);
        }
    }
}

BOOL WINAPI SHIsBadInterfacePtr(LPCVOID pv, UINT cbVtbl)
{
    IUnknown const* punk = pv;
    return(IsBadReadPtr(punk, sizeof(punk->lpVtbl))
        || IsBadReadPtr(punk->lpVtbl, cbVtbl)
        || IsBadCodePtr((FARPROC)punk->lpVtbl->Release));
}


int WINAPI StrToOleStrN(LPOLESTR pwsz, int cchWideChar, LPCTSTR psz, int cchMultiByte)
{
#ifdef UNICODE
    int cchOutput;
    VDATEINPUTBUF(pwsz, OLECHAR, cchWideChar);

    if (cchWideChar > cchMultiByte)
    {
        cchWideChar = cchMultiByte;
    }
    cchOutput = cchWideChar;

    // HACK: Ignore DBCS for now
    while (--cchWideChar)
    {
        if ((*pwsz++ = *psz++) == 0)
            return(cchOutput - cchWideChar + 1);
    };

    *pwsz = 0;

    return(cchOutput);
#else
    return MultiByteToWideChar(CP_ACP, 0, psz, cchMultiByte, pwsz, cchWideChar);
#endif
}

UNALIGNED WCHAR* ualstrcpynUnimpl(UNALIGNED WCHAR* lpString1, UNALIGNED const WCHAR* lpString2, int iMaxLength)
{
    UNALIGNED WCHAR* src;
    UNALIGNED WCHAR* dst;

    src = (UNALIGNED WCHAR*)lpString2;
    dst = lpString1;

    while (iMaxLength && *src)
    {
        *dst++ = *src++;
        iMaxLength--;
    }

    if (iMaxLength)
    {
        *dst = '\0';
    }
    else
    {
        dst--;
        *dst = '\0';
    }
    return dst;
}

UNALIGNED WCHAR* ualstrcpyUnimpl(UNALIGNED WCHAR* dst, UNALIGNED const WCHAR* src)
{
    UNALIGNED WCHAR* cp = dst;

    while (*cp++ = *src++)
        NULL;

    return(dst);
}
HDSA g_hdsaDropTargets = NULL;
LPDROPTARGETINFO SHDrag_FindDropTarget(HWND hwndTarget, int* pi, BOOL fInContext)
{
    int i;
    UINT idProcess = GetCurrentProcessId();
    UINT idThread = GetCurrentThreadId();

    if (!g_hdsaDropTargets) {
        return NULL;    // No drop target registered ever.
    }

    for (i = 0; i < DSA_GetItemCount(g_hdsaDropTargets); i++)
    {
        LPDROPTARGETINFO pdti = DSA_GetItemPtr(g_hdsaDropTargets, i);
        if (pdti->hwndTarget == hwndTarget)
        {
            if (fInContext && pdti->idProcess != idProcess && pdti->idThread != idThread)
            {
                //
                // WANTED: I've been looking for a repro case which will
                // hit this assert. Please let me know if you hit this.
                // (SatoNa)
                //
                //Assert(0);
                continue;
            }

            if (pi) {
                *pi = i;
            }
            return pdti;
        }
    }
    return NULL;
}
   
BOOL WINAPI StrRetToStrN(LPTSTR szOut, UINT uszOut, LPSTRRET pStrRet, LPCITEMIDLIST pidl)
{
    switch (pStrRet->uType)
    {
#ifdef UNICODE
        // We will be returning a UNICODE string

    case STRRET_OLESTR:
    {
        LPTSTR  pszStr;
        pszStr = pStrRet->pOleStr;

        lstrcpyn(szOut, pStrRet->pOleStr, uszOut);
        SHFree(pszStr);
    }
    break;

    case STRRET_CSTR:
        MultiByteToWideChar(CP_ACP, 0, pStrRet->cStr, -1, szOut, uszOut);
        break;

    case STRRET_OFFSET:
        if (pidl)
        {
            // BUGBUG (DavePl) Alignment problems here

            MultiByteToWideChar(CP_ACP, 0, STRRET_OFFPTR(pidl, pStrRet), -1,
                szOut, uszOut);
            break;
        }
        goto punt;
#else
        // We will be returning an ANSI/DBCS string

    case STRRET_OLESTR:
        WideCharToMultiByte(CP_ACP, 0, pStrRet->pOleStr, -1, szOut, uszOut,
            NULL, NULL);
        SHFree(pStrRet->pOleStr);
        break;

    case STRRET_CSTR:
        lstrcpyn(szOut, pStrRet->cStr, uszOut);
        break;

    case STRRET_OFFSET:
        if (pidl)
        {
            ualstrcpynUnimpl(szOut, STRRET_OFFPTR(pidl, pStrRet), uszOut);
            break;
        }
        goto punt;
#endif

    default:
        ////Assert(FALSE && "Bad STRRET uType");
    punt:
        if (uszOut)
        {
            *szOut = TEXT('\0');
        }
        return(FALSE);
    }

    return(TRUE);
}


HRESULT WINAPI SHRevokeDragDrop(HWND hwnd)
{
    HRESULT hres = DRAGDROP_E_NOTREGISTERED;
    DROPTARGETINFO dti; // No initialization needed (CAREFUL!)

    //
    // Find the registered one, and remove it from the list.
    //
    {
        int i;
        LPDROPTARGETINFO pdti = SHDrag_FindDropTarget(hwnd, &i, TRUE);
        if (pdti)
        {
            dti = *pdti;    // copy it for later (see below)
            DSA_DeleteItem(g_hdsaDropTargets, i);
            hres = NOERROR;
        }
    }

    //
    // We need to do this clean-up from outside of the critical section.
    //
    if (SUCCEEDED(hres))
    {
        dti.pdtgt->lpVtbl->Release(dti.pdtgt);
        DestroyWindow(dti.hwndProxyTarget);
#ifdef SN_TRACE
        DebugMsg(DM_TRACE, TEXT("sh TR - A drop target revoked (%x, %x, %x) - Total %d"),
            dti.hwndTarget, dti.hwndProxyTarget, dti.pdtgt,
            DSA_GetItemCount(g_hdsaDropTargets));
#endif

#ifdef OLE_DELAYED_LOADING
        if (dti.fRegisteredToOLE)
        {
            HRESULT hresT;
            hresT = SHXRevokeDragDrop(hwnd);
            //Assert(SUCCEEDED(hresT));   // something is wrong with OLE
        }
#endif // OLE_DELAYED_LOADING
    }

    return hres;
}

LRESULT TargetProxy_OnDragDropUnimpl(LPDRAGCONTEXT pdrgc, WPARAM wParam)
{
    return;
}

LRESULT CALLBACK TargetProxyWndProcUnimpl(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case PXM_DRAGDROP:
        return TargetProxy_OnDragDropUnimpl((LPDRAGCONTEXT)lParam, wParam);

    case WM_CANCELMODE:
        //
        //  This is one of methods that MikeSch suggested
        // me to prevent loosing capture during this loop.
        //
        // Swallow it to prevent loosing the capture.
        return 0L;

#ifdef OLE_DELAYED_LOADING
    case PXM_FOUNDOLE:
        return TargetProxy_OnFoundOLE(hwnd, (HWND)wParam);
#endif OLE_DELAYED_LOADING
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

HWND _CreateProxyWindowUnimpl()
{
    WNDCLASS cls;

    if (!GetClassInfo(HINST_THISDLL, TEXT("ProxyTarget"), &cls))
    {
        cls.hCursor = NULL;
        cls.hIcon = NULL;
        cls.lpszMenuName = NULL;
        cls.hInstance = HINST_THISDLL;
        cls.lpszClassName = TEXT("ProxyTarget");
        cls.hbrBackground = NULL;
        cls.lpfnWndProc = TargetProxyWndProcUnimpl;
        cls.style = 0;
        cls.cbWndExtra = 0;
        cls.cbClsExtra = 0;

        RegisterClass(&cls);
    }

    return CreateWindowEx(0, TEXT("ProxyTarget"), NULL,
        WS_OVERLAPPED, 0, 0, 0, 0, NULL, NULL, HINST_THISDLL, NULL);
}

int g_bAnyDropTarget = 0;
HRESULT WINAPI SHRegisterDragDrop(HWND hwnd, LPDROPTARGET pdtgt)
{
    HRESULT hres;
    HWND hwndProxyTarget = _CreateProxyWindowUnimpl();
    DROPTARGETINFO dti = { hwnd, hwndProxyTarget,
                           pdtgt,
                           GetCurrentProcessId(),
                           GetCurrentThreadId(),
                           FALSE };

#ifdef OLE_DELAYED_LOADING
    //
    // If the OLE is already loaded in this process, register this window
    // to the OLE as well. Note that we do it before so that we can store
    // fRegisteredToOLE flag in the g_hdsaDropTargets.
    //
    if (g_fRegisterToOLE)
    {
        SHXRegisterDragDrop(hwnd, pdtgt);
        dti.fRegisteredToOLE = TRUE;
    }
#endif // OLE_DELAYED_LOADING

    pdtgt->lpVtbl->AddRef(pdtgt);

    {
        if (!g_hdsaDropTargets) {
            g_hdsaDropTargets = DSA_Create(sizeof(DROPTARGETINFO), 8);
        }

        if (g_hdsaDropTargets)
        {
            if (!SHDrag_FindDropTarget(hwnd, NULL, FALSE))
            {
                DSA_InsertItem(g_hdsaDropTargets, 0x7fffffff, &dti);
                g_bAnyDropTarget = TRUE;
                hres = S_OK;
            }
            else
            {
                //
                // If the window is already registered, we'll hit this assert;
                // we are not supposed to hit this assert. Please let me know
                // if you hit this assert. (SatoNa)
                //
                ////Assert(0);
                hres = DRAGDROP_E_ALREADYREGISTERED;
            }
        }
        else
        {
            hres = E_OUTOFMEMORY;
        }
    }

#ifdef SN_TRACE
    DebugMsg(DM_TRACE, TEXT("sh TR - A drop target registered (%x, %x, %x) - Total %d"),
        hwnd, hwndProxyTarget, pdtgt,
        DSA_GetItemCount(g_hdsaDropTargets));
#endif

    return hres;
}

int WINAPI SHOutOfMemoryMessageBox(HWND hwndOwner, LPTSTR pszTitle, UINT fuStyle)
{
    LPTSTR g_pszOutOfMemory = 0;
    TCHAR szTitle[128];
    int ret;

    szTitle[0] = TEXT('\0');

    if (pszTitle == NULL)
    {
        GetWindowText(hwndOwner, szTitle, ARRAYSIZE(szTitle));
        pszTitle = szTitle;
    }

    ret = MessageBox(hwndOwner, g_pszOutOfMemory, pszTitle, fuStyle | MB_SETFOREGROUND);
    if (ret == -1)
    {
        ret = MessageBox(hwndOwner, g_pszOutOfMemory, pszTitle, fuStyle | MB_SYSTEMMODAL);
    }

    return ret;
}

HANDLE MapHandle(HANDLE hData, DWORD dwSource, DWORD dwDest, DWORD dwDesiredAccess, DWORD dwFlags)
{
    HANDLE hSource = NULL;
    HANDLE hDest = NULL;
    HANDLE hNew = NULL;
    BOOL fOk;

    if (dwSource == GetCurrentProcessId())
        hSource = GetCurrentProcess();
    else
        hSource = OpenProcess(PROCESS_DUP_HANDLE, FALSE, dwSource);

    if (!hSource)
        goto DoExit;

    if (dwDest == GetCurrentProcessId())
        hDest = GetCurrentProcess();
    else
        hDest = OpenProcess(PROCESS_DUP_HANDLE, FALSE, dwDest);

    if (!hDest)
        goto DoExit;

    fOk = DuplicateHandle(hSource, hData,
        hDest, &hNew,
        dwDesiredAccess,
        FALSE, dwFlags | DUPLICATE_SAME_ACCESS);
    if (!fOk)
        hNew = (HANDLE)NULL;

DoExit:
    if (hSource && dwSource != GetCurrentProcessId())
        CloseHandle(hSource);

    if (hDest && dwDest != GetCurrentProcessId())
        CloseHandle(hDest);

    return hNew;
}

//LPVOID SHLockSharedUnimpl(HANDLE  hData,DWORD   dwSourceProcessId) 
//{
//    LPSHMAPHEADER   lpmh;
//    HANDLE          hUsableData;
//
//    hUsableData = MapHandle(hData, dwSourceProcessId, GetCurrentProcessId(), FILE_MAP_ALL_ACCESS, 0);
//
//    //
//    // Now map that new process specific handle and close it
//    //
//    lpmh = MapViewOfFile(hUsableData, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
//    CloseHandle(hUsableData);
//
//    if (!lpmh)
//        return NULL;
//
//    return (LPVOID)(lpmh + 1);
//}
//
//BOOL SHUnlockSharedUnimpl(LPVOID  lpvData) 
//{
//    LPSHMAPHEADER lpmh = (LPSHMAPHEADER)lpvData;
//
//    //
//    // Now just unmap the view of the file
//    //
//    return UnmapViewOfFile(lpmh - 1);
//}
//
//HANDLE SHAllocSharedUnimpl(LPCVOID lpvData, DWORD   dwSize, DWORD   dwDestinationProcessId) 
//{
//    HANDLE  hData;
//    LPSHMAPHEADER lpmh;
//    HANDLE hUsableData;
//
//    //
//    // Make a filemapping handle with this data in it.
//    //
//    hData = CreateFileMapping((HANDLE)-1, NULL, PAGE_READWRITE, 0,
//        dwSize + sizeof(SHMAPHEADER), NULL);
//    if (hData == NULL)
//    {
//        // DebugMsg...
//        return NULL;
//    }
//
//    lpmh = MapViewOfFile(hData, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
//    if (!lpmh)
//    {
//        // DebugMsg...
//        CloseHandle(hData);
//        return NULL;
//    }
//    lpmh->dwSize = dwSize;
//
//    if (lpvData)
//        memcpy((LPVOID)(lpmh + 1), lpvData, dwSize);
//
//    UnmapViewOfFile(lpmh);
//
//    hUsableData = MapHandle(hData,
//        GetCurrentProcessId(),
//        dwDestinationProcessId,
//        FILE_MAP_ALL_ACCESS,
//        DUPLICATE_CLOSE_SOURCE);
//    if (!hUsableData)
//    {
//        CloseHandle(hData);
//    }
//
//    return hUsableData;
//}
//
//BOOL SHFreeSharedUnimpl(HANDLE hData, DWORD dwSourceProcessId)
//{
//    HANDLE hUsableData;
//
//    //
//    // The below call closes the original handle in whatever process it
//    // came from.
//    //
//    hUsableData = MapHandle(hData, dwSourceProcessId,
//        GetCurrentProcessId(),
//        FILE_MAP_ALL_ACCESS, DUPLICATE_CLOSE_SOURCE);
//
//    //
//    // Now free up the local handle
//    //
//    return CloseHandle(hUsableData);
//}

LPTSTR WINAPI PathFindFileNameUnimpl(LPCTSTR pPath)
{
    LPCTSTR pT;

    for (pT = pPath; *pPath; pPath = CharNext(pPath)) {
        if ((pPath[0] == TEXT('\\') || pPath[0] == TEXT(':')) && pPath[1] && (pPath[1] != TEXT('\\')))
            pT = pPath + 1;
    }

    return (LPTSTR)pT;   // const -> non const
}

void WINAPI GetEffectiveClientRectUnimpl(HWND hWnd, LPRECT lprc, LPINT lpInfo)
{
    RECT rc;
    HWND hCtl;

    GetClientRect(hWnd, lprc);

    /* Get past the menu
     */
    for (lpInfo += 2; lpInfo[0]; lpInfo += 2)
    {
        hCtl = GetDlgItem(hWnd, lpInfo[1]);
        /* We check the style bit because the parent window may not be visible
         * yet (still in the create message)
         */
        if (!hCtl || !(GetWindowStyle(hCtl) & WS_VISIBLE))
            continue;

        GetWindowRect(hCtl, &rc);
        ScreenToClient(hWnd, (LPPOINT)&rc);
        ScreenToClient(hWnd, ((LPPOINT)&rc) + 1);

        SubtractRect(lprc, lprc, &rc);
    }
}

BOOL  WINAPI PathIsRootUnimpl(LPCTSTR pPath)
{
    if (!IsDBCSLeadByte(*pPath))
    {
        if (!lstrcmpi(pPath + 1, c_szColonSlash))                  // "X:\" case
            return TRUE;
    }

    if ((*pPath == TEXT('\\')) && (*(pPath + 1) == 0))        // "\" case
        return TRUE;

    if (DBL_BSLASH(pPath))      // smells like UNC name
    {
        LPCTSTR p;
        int cBackslashes = 0;

        for (p = pPath + 2; *p; p = CharNext(p)) {
            if (*p == TEXT('\\') && (++cBackslashes > 1))
                return FALSE;   /* not a bare UNC name, therefore not a root dir */
        }
        return TRUE;    /* end of string with only 1 more backslash */
        /* must be a bare UNC, which looks like a root dir */
    }
    return FALSE;
}

BOOL WINAPI ImageList_GetIconSizeUnimpl(IMAGELIST* piml, int FAR* cx, int FAR* cy)
{
    V_HIMAGELIST(piml);

    if (!cx || !cy)
        return FALSE;

    *cx = piml->cx;
    *cy = piml->cy;
    return TRUE;
}

BOOL WINAPI SHWinHelpUnimpl(HWND hwndMain, LPCTSTR lpszHelp, UINT usCommand, DWORD ulData)
{
    // Try to show help
    if (!WinHelp(hwndMain, lpszHelp, usCommand, ulData))
    {
        // Problem.
        ShellMessageBox(g_hinst, hwndMain,
            MAKEINTRESOURCE(IDS_WINHELPERROR),
            MAKEINTRESOURCE(IDS_WINHELPTITLE),
            MB_ICONHAND | MB_OK);
        return FALSE;
    }
    return TRUE;
}

BOOL WINAPI PathIsRelativeUnimpl(LPCTSTR lpszPath)
{
    // The NULL path is assumed relative
    if (*lpszPath == 0)
        return TRUE;

    // Does it begin with a slash ?
    if (lpszPath[0] == TEXT('\\'))
        return FALSE;
    // Does it begin with a drive and a colon ?
    else if (!IsDBCSLeadByte(lpszPath[0]) && lpszPath[1] == TEXT(':'))
        return FALSE;
    // Probably relative.
    else
        return TRUE;
}

int WINAPI PathGetDriveNumberUnimpl(LPCTSTR lpsz)
{
    if (!IsDBCSLeadByte(lpsz[0]) && lpsz[1] == TEXT(':'))
    {
        if (lpsz[0] >= TEXT('a') && lpsz[0] <= TEXT('z'))
            return (lpsz[0] - TEXT('a'));
        else if (lpsz[0] >= TEXT('A') && lpsz[0] <= TEXT('Z'))
            return (lpsz[0] - TEXT('A'));
    }
    return -1;
}

LPTSTR WINAPI PathFindExtensionUnimpl(LPCTSTR pszPath)
{
    LPCTSTR pszDot;

    for (pszDot = NULL; *pszPath; pszPath = CharNext(pszPath))
    {
        switch (*pszPath) {
        case TEXT('.'):
            pszDot = pszPath;         // remember the last dot
            break;
        case TEXT('\\'):
        case TEXT(' '):         // extensions can't have spaces
            pszDot = NULL;       // forget last dot, it was in a directory
            break;
        }
    }

    // if we found the extension, return ptr to the dot, else
    // ptr to end of the string (NULL extension) (cast->non const)
    return pszDot ? (LPTSTR)pszDot : (LPTSTR)pszPath;
}

// If a path is contained in quotes then remove them.
void WINAPI PathUnquoteSpacesUnimpl(LPTSTR lpsz)
{
    int cch;

    cch = lstrlen(lpsz);

    // Are the first and last chars quotes?
    if (lpsz[0] == TEXT('"') && lpsz[cch - 1] == TEXT('"'))
    {
        // Yep, remove them.
        lpsz[cch - 1] = TEXT('\0');
        memcpy(lpsz, lpsz + 1, (cch - 1) * sizeof(TCHAR));
    }
}

BOOL WINAPI PathRemoveFileSpecUnimpl(LPTSTR pFile)
{
    LPTSTR pT;
    LPTSTR pT2 = pFile;

    for (pT = pT2; *pT2; pT2 = CharNext(pT2)) {
        if (*pT2 == TEXT('\\'))
            pT = pT2;             // last "\" found, (we will strip here)
        else if (*pT2 == TEXT(':')) {   // skip ":\" so we don't
            if (pT2[1] == TEXT('\\'))    // strip the "\" from "C:\"
                pT2++;
            pT = pT2 + 1;
        }
    }
    if (*pT == 0)
        return FALSE;   // didn't strip anything

    //
    // handle the \foo case
    //
    else if ((pT == pFile) && (*pT == TEXT('\\'))) {
        // Is it just a '\'?
        if (*(pT + 1) != TEXT('\0')) {
            // Nope.
            *(pT + 1) = TEXT('\0');
            return TRUE;        // stripped something
        }
        else {
            // Yep.
            return FALSE;
        }
    }
    else {
        *pT = 0;
        return TRUE;    // stripped something
    }
}

void PathRemoveExtensionUnimpl(LPTSTR pszPath)
{
    LPTSTR pExt = PathFindExtensionUnimpl(pszPath);
    if (*pExt)
    {
        ////Assert(*pExt == TEXT('.'));
        *pExt = 0;    // null out the "."
    }
}

void WINAPI PathRemoveBlanksUnimpl(LPTSTR lpszString)
{
    LPTSTR lpszPosn = lpszString;
    /* strip leading blanks */
    while (*lpszPosn == TEXT(' ')) {
        lpszPosn++;
    }
    if (lpszPosn != lpszString)
        lstrcpy(lpszString, lpszPosn);

    /* strip trailing blanks */

    // Find the last non-space
    // Note that AnsiPrev is cheap is non-DBCS, but very expensive otherwise
    for (lpszPosn = lpszString; *lpszString; lpszString = CharNext(lpszString))
    {
        if (*lpszString != TEXT(' '))
        {
            lpszPosn = lpszString;
        }
    }

    // Note AnsiNext is a macro for non-DBCS, so it will not stop at NULL
    if (*lpszPosn)
    {
        *CharNext(lpszPosn) = TEXT('\0');
    }
}

void PathRemoveArgsUnimpl(LPTSTR pszPath)
{
    LPTSTR pArgs = PathGetArgsUnimpl(pszPath);
    if (*pArgs)
        *(pArgs - 1) = TEXT('\0');   // clobber the ' '
    // Handle trailing space.
    else
    {
        pArgs = CharPrev(pszPath, pArgs);
        if (*pArgs == TEXT(' '))
            *pArgs = TEXT('\0');
    }
}



int WINAPI PathParseIconLocationUnimpl(LPTSTR pszIconFile)
{
    int iIndex = 0;
    LPTSTR pszComma = StrChr(pszIconFile, TEXT(','));

    if (SELECTOROF(pszComma)) {
        *pszComma++ = 0;            // terminate the icon file name.
        while (*pszComma == TEXT(' ')) *pszComma++; // skip spaces
        iIndex = StrToInt(pszComma);
    }
    PathRemoveBlanksUnimpl(pszIconFile);
    return iIndex;
}

__inline BOOL PathIsUNCUnimpl(LPWSTR psz)
{
    return (psz[0] == L'\\' && psz[1] == L'\\');
}

DWORD WINAPI PathIsUNCServerShareUnimpl(LPCTSTR pszPath)
{
    int i = 0;
    LPCTSTR pszTmp;
    DWORD dwRes = 0;

    if (!pszPath || !(*pszPath))
    {
        return dwRes;
    }


    // Did the path start with "\\"
    if (DBL_BSLASH(pszPath))
    {
        // walk the string...
        for (pszTmp = pszPath; pszTmp && *pszTmp; pszTmp++)
        {
            if (*pszTmp == TEXT('\\'))
            {
                i++;
            }
        }

        // Only two slashes means \\server
        if (i == 2)
        {
            dwRes = 2;
        }

        // Three slashes means \\server\share
        else if (i == 3)
        {
            dwRes = 3;
        }

        // It's just UNC path...
        else
        {
            dwRes = 1;
        }
    }


    return dwRes;
}

BOOL WINAPI PathIsDirectoryUnimpl(LPCTSTR pszPath)
{
    DWORD dwAttribs;
    DWORD dwPathType = PathIsUNCServerShareUnimpl(pszPath);

    if (dwPathType == 2)
    {
        // Be Win95 compatible in our error code
        SetLastError(ERROR_BAD_PATHNAME);
    }
    else
    {
        dwAttribs = GetFileAttributes(pszPath);
        if (dwAttribs != (DWORD)-1)
            return (BOOL)(dwAttribs & FILE_ATTRIBUTE_DIRECTORY);
    }

    return FALSE;
}

LPTSTR WINAPI PathGetArgsUnimpl(LPCTSTR pszPath)
{
    BOOL fInQuotes = FALSE;

    if (!pszPath)
        return NULL;

    while (*pszPath)
    {
        if (*pszPath == TEXT('"'))
            fInQuotes = !fInQuotes;
        else if (!fInQuotes && *pszPath == TEXT(' '))
            return (LPTSTR)pszPath + 1;
        pszPath = CharNext(pszPath);
    }

    return (LPTSTR)pszPath;
}

BOOL WINAPI PathFileExistsUnimpl(LPCTSTR lpszPath)
{
    DWORD dwErrMode;
    BOOL fResult = FALSE;

    if (!lpszPath || !(*lpszPath))
    {
        return fResult;
    }

    dwErrMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    fResult = ((UINT)GetFileAttributes(lpszPath) != (UINT)-1);

    SetErrorMode(dwErrMode);

    return fResult;

}

BOOL PathStripToRootUnimpl(LPTSTR szRoot)
{
    while (!PathIsRootUnimpl(szRoot))
    {
        if (!PathRemoveFileSpecUnimpl(szRoot))
        {
            // If we didn't strip anything off,
            // must be current drive
            return(FALSE);
        }
    }

    return(TRUE);
}

void NearRootFixupsUnimpl(LPTSTR lpszPath, BOOL fUNC)
{
    // Check for empty path.
    if (lpszPath[0] == TEXT('\0'))
    {
        // Fix up.
        lpszPath[0] = TEXT('\\');
        lpszPath[1] = TEXT('\0');
    }
    // Check for missing slash.
    if (!IsDBCSLeadByte(lpszPath[0]) && lpszPath[1] == TEXT(':') && lpszPath[2] == TEXT('\0'))
    {
        // Fix up.
        lpszPath[2] = TEXT('\\');
        lpszPath[3] = TEXT('\0');
    }
    // Check for UNC root.
    if (fUNC && lpszPath[0] == TEXT('\\') && lpszPath[1] == TEXT('\0'))
    {
        // Fix up.
        lpszPath[0] = TEXT('\\');
        lpszPath[1] = TEXT('\\');
        lpszPath[2] = TEXT('\0');
    }
}

// Canonicalizes a path.
BOOL PathCanonicalizeUnimpl(LPTSTR lpszDst, LPCTSTR lpszSrc)
{
    LPCTSTR lpchSrc;
    LPCTSTR lpchPCEnd;           // Pointer to end of path component.
    LPTSTR lpchDst;
    BOOL fUNC;
    int cbPC;

    fUNC = PathIsUNCUnimpl(lpszSrc);    // Check for UNCness.

    // Init.
    lpchSrc = lpszSrc;
    lpchDst = lpszDst;

    while (*lpchSrc)
    {
        // REVIEW: this should just return the count
        lpchPCEnd = GetPCEnd(lpchSrc);
        cbPC = (lpchPCEnd - lpchSrc) + 1;

        // Check for slashes.
        if (cbPC == 1 && *lpchSrc == TEXT('\\'))
        {
            // Just copy them.
            *lpchDst = TEXT('\\');
            lpchDst++;
            lpchSrc++;
        }
        // Check for dots.
        else if (cbPC == 2 && *lpchSrc == TEXT('.'))
        {
            // Skip it...
            // Are we at the end?
            if (*(lpchSrc + 1) == TEXT('\0'))
            {
                lpchDst--;
                lpchSrc++;
            }
            else
                lpchSrc += 2;
        }
        // Check for dot dot.
        else if (cbPC == 3 && *lpchSrc == TEXT('.') && *(lpchSrc + 1) == TEXT('.'))
        {
            // make sure we aren't already at the root
            if (!PathIsRootUnimpl(lpszDst))
            {
                // Go up... Remove the previous path component.
                lpchDst = (LPTSTR)PCStart(lpszDst, lpchDst - 1);
            }
            else
            {
                // When we can't back up, remove the trailing backslash
                // so we don't copy one again. (C:\..\FOO would otherwise
                // turn into C:\\FOO).
                if (*(lpchSrc + 2) == TEXT('\\'))
                {
                    lpchSrc++;
                }
            }
            lpchSrc += 2;       // skip ".."
        }
        // Everything else
        else
        {
            // Just copy it.
            lstrcpyn(lpchDst, lpchSrc, cbPC);
            lpchDst += cbPC - 1;
            lpchSrc += cbPC - 1;
        }
        // Keep everything nice and tidy.
        *lpchDst = TEXT('\0');
    }

    // Check for weirdo root directory stuff.
    NearRootFixupsUnimpl(lpszDst, fUNC);

    return TRUE;
}

size_t ualstrlenW(UNALIGNED const WCHAR* wcs)
{
    UNALIGNED const WCHAR* eos = wcs;

    while (*eos++)
    {
        NULL;
    }

    return((size_t)(eos - wcs - 1));
}


LPTSTR WINAPI PathCombineUnimpl(LPTSTR lpszDest, LPCTSTR lpszDir, LPCTSTR lpszFile)
{
    TCHAR szTemp[MAX_PATH];
    LPTSTR pszT;

    if (!lpszFile || *lpszFile == TEXT('\0')) {

        ualstrcpynUnimpl(szTemp, lpszDir, ARRAYSIZE(szTemp));       // lpszFile is empty

    }
    else if (lpszDir && *lpszDir && PathIsRelativeUnimpl(lpszFile)) {

        ualstrcpynUnimpl(szTemp, lpszDir, ARRAYSIZE(szTemp));
        pszT = PathAddBackslashUnimpl(szTemp);
        if (pszT) {
            int iLen = lstrlen(szTemp);
            if ((iLen + ualstrlenW(lpszFile)) < ARRAYSIZE(szTemp)) {
                ualstrcpyUnimpl(pszT, lpszFile);
            }
            else
                return NULL;
        }
        else
            return NULL;

    }
    else if (lpszDir && *lpszDir &&
        *lpszFile == TEXT('\\') && !PathIsUNCUnimpl(lpszFile)) {

        ualstrcpynUnimpl(szTemp, lpszDir, ARRAYSIZE(szTemp));
        // BUGBUG: Note that we do not check that an actual root is returned;
        // it is assumed that we are given valid parameters
        PathStripToRootUnimpl(szTemp);

        pszT = PathAddBackslashUnimpl(szTemp);
        if (pszT)
        {
            // Skip the backslash when copying
            ualstrcpynUnimpl(pszT, lpszFile + 1, ARRAYSIZE(szTemp) - 1 - (pszT - szTemp));
        }
        else
            return NULL;

    }
    else {

        ualstrcpynUnimpl(szTemp, lpszFile, ARRAYSIZE(szTemp));     // already fully qualified file part

    }

    PathCanonicalizeUnimpl(lpszDest, szTemp); // this deals with .. and . stuff

    return lpszDest;
}

LPSTR WINAPI PathBuildRootUnimpl(LPSTR szRoot, int iDrive)
{
    ////Assert(iDrive >= 0 && iDrive < 26);

    szRoot[0] = (TCHAR)iDrive + (TCHAR)TEXT('A');
    szRoot[1] = TEXT(':');
    szRoot[2] = TEXT('\\');
    szRoot[3] = 0;

    return szRoot;
}

BOOL WINAPI PathAppendUnimpl(LPTSTR pPath, LPCTSTR pMore)
{

    /* Skip any initial terminators on input. */

#ifndef UNICODE

    while (*pMore == TEXT('\\'))
        pMore = CharNext(pMore);
#else

    while (*pMore == TEXT('\\'))
        pMore++;

#endif

    return (BOOL)PathCombineUnimpl(pPath, pPath, pMore);
}

LPTSTR WINAPI PathAddBackslashUnimpl(LPTSTR lpszPath)
{
    LPTSTR lpszEnd;

    // try to keep us from tromping over MAX_PATH in size.
    // if we find these cases, return NULL.  Note: We need to
    // check those places that call us to handle their GP fault
    // if they try to use the NULL!
    int ichPath = lstrlen(lpszPath);
    if (ichPath >= (MAX_PATH - 1))
    {
        ////Assert(FALSE);      // Let the caller know!
        return(NULL);
    }

    lpszEnd = lpszPath + ichPath;

    // this is really an error, caller shouldn't pass
    // an empty string
    if (!*lpszPath)
        return lpszEnd;

    /* Get the end of the source directory
    */
    switch (*CharPrev(lpszPath, lpszEnd)) {
    case TEXT('\\'):
        break;

    default:
        *lpszEnd++ = TEXT('\\');
        *lpszEnd = TEXT('\0');
    }
    return lpszEnd;
}

LONG WINAPI PathProcessCommandUnimpl(LPCTSTR lpSrc, LPTSTR lpDest, int iDestMax, DWORD dwFlags)
{
    TCHAR szName[MAX_PATH];
    LPTSTR lpBuffer, lpBuffer2;
    LPCTSTR lpArgs = NULL;
    DWORD dwAttrib;
    LONG i, iTotal;
    LONG iResult = -1;
    BOOL bAddQuotes = FALSE;
    BOOL bQualify = FALSE;
    BOOL bFound = FALSE;
    BOOL bHitSpace = FALSE;

    // Process the given source string, attempting to find what is that path, and what is its
    // arguments.

    if (lpSrc)
    {
        // Extract the sub string, if its is realative then resolve (if required).

        if (*lpSrc == TEXT('\"'))
        {
            for (lpSrc++, i = 0; i < MAX_PATH && *lpSrc && *lpSrc != TEXT('\"'); i++, lpSrc++)
                szName[i] = *lpSrc;

            szName[i] = TEXT('\0');

            if (*lpSrc)
                lpArgs = lpSrc + 1;

            if (((dwFlags & PPCF_FORCEQUALIFY) || PathIsRelativeUnimpl(szName))
                && !(dwFlags & PPCF_NORELATIVEOBJECTQUALIFY))
            {
                if (!PathResolve(szName, NULL, PRF_TRYPROGRAMEXTENSIONS))
                    goto exit_gracefully;
            }

            bFound = TRUE;
        }
        else
        {
            // Is this a relative object, and then take each element upto a seperator
            // and see if we hit an file system object.  If not then we can

            bQualify = PathIsRelativeUnimpl(lpSrc) || ((dwFlags & PPCF_FORCEQUALIFY) != 0);

            for (i = 0; i < MAX_PATH; i++)
            {
                szName[i] = lpSrc[i];

                // If we hit a space then the string either contains a LFN or we have
                // some arguments.  Therefore attempt to get the attributes for the string
                // we have so far, if we are unable to then we can continue
                // checking, if we hit then we know that the object exists and the
                // trailing string are its arguments.

                if (!szName[i] || szName[i] == TEXT(' '))
                {
                    szName[i] = TEXT('\0');

                    while (TRUE)
                    {
                        if (bQualify && !PathResolve(szName, NULL, PRF_TRYPROGRAMEXTENSIONS))
                            break;

                        dwAttrib = GetFileAttributes(szName);

                        if (dwAttrib == -1 || ((dwAttrib & FILE_ATTRIBUTE_DIRECTORY) && dwFlags & PPCF_NODIRECTORIES))
                            break;

                        if (bQualify && (dwFlags & PPCF_NORELATIVEOBJECTQUALIFY))
                            *lstrcpyn(szName, lpSrc, i) = TEXT(' ');

                        bFound = TRUE;                  // success
                        lpArgs = &lpSrc[i];

                        goto exit_gracefully;
                    }

                    if (bQualify)
                        memcpy(szName, lpSrc, (i + 1) * sizeof(TCHAR));
                    else
                        szName[i] = lpSrc[i];

                    bHitSpace = TRUE;
                }

                if (!szName[i])
                    break;
            }
        }
    }

exit_gracefully:

    // Work out how big the temporary buffer should be, allocate it and
    // build the returning string into it.  Then compose the string
    // to be returned.

    if (bFound)
    {
        if (StrChr(szName, TEXT(' ')))
            bAddQuotes = dwFlags & PPCF_ADDQUOTES;

        iTotal = lstrlen(szName) + 1;                // for terminator
        iTotal += bAddQuotes ? 2 : 0;
        iTotal += (dwFlags & PPCF_ADDARGUMENTS) && lpArgs ? lstrlen(lpArgs) : 0;

        if (lpDest)
        {
            if (iTotal <= iDestMax)
            {
                lpBuffer = lpBuffer2 = (LPTSTR)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * iTotal);

                if (lpBuffer)
                {
                    // First quote if required
                    if (bAddQuotes)
                        *lpBuffer2++ = TEXT('\"');

                    // Matching name
                    lstrcpy(lpBuffer2, szName);

                    // Closing quote if required
                    if (bAddQuotes)
                        lstrcat(lpBuffer2, TEXT("\""));

                    // Arguments (if requested)
                    if ((dwFlags & PPCF_ADDARGUMENTS) && lpArgs)
                        lstrcat(lpBuffer2, lpArgs);

                    // Then copy into callers buffer, and free out temporary buffer
                    lstrcpy(lpDest, lpBuffer);
                    LocalFree((HGLOBAL)lpBuffer);

                    // Return the length of the resulting string
                    iResult = iTotal;
                }
            }
        }
        else
        {
            // Resulting string is this big, although nothing returned (allows them to allocate a buffer)
            iResult = iTotal;
        }
    }

    return iResult;
}

BOOL WINAPI RLEnumRegistry(HDPA hdpa, PRLCALLBACK prlcb,
    LPCTSTR pszSource, LPCTSTR pszDest)
{
    static const TCHAR c_szWin16[] = TEXT("Win16");
    static const TCHAR c_szWin32[] = TEXT("Win32");
    static const TCHAR c_szHelpDir[] = TEXT("HelpDir");
    static const TCHAR c_szTypeLib[] = TEXT("TypeLib");
    HKEY hkeyRoot;
    HKEY hkeySecond;
    HKEY hkey3;
    TCHAR szRootName[80];
    TCHAR szSecond[80];
    int  iRootName;
    int  iSecond;
    int iThird;
    long cbValue;

    TCHAR szPath[MAX_PATH];

    // First look in the CLSID section. I dont think they
    // need to be localized.
    if (RegOpenKey(HKEY_CLASSES_ROOT, TEXT("CLSID"), &hkeyRoot) == ERROR_SUCCESS)
    {
        for (iRootName = 0; RegEnumKey(hkeyRoot, iRootName,
            szRootName, ARRAYSIZE(szRootName)) == ERROR_SUCCESS; iRootName++)
        {
            // Now try to enum this key
            if (RegOpenKey(hkeyRoot, szRootName, &hkeySecond) == ERROR_SUCCESS)
            {
                for (iSecond = 0; RegEnumKey(hkeySecond, iSecond,
                    szSecond, ARRAYSIZE(szSecond)) == ERROR_SUCCESS; iSecond++)
                {
                    // Now see if this is one of the keys that we know
                    // contain paths of interest.
                    if ((lstrcmpi(szSecond, TEXT("DefaultIcon")) == 0) ||
                        (lstrcmpi(szSecond, TEXT("LocalServer")) == 0) ||
                        (lstrcmpi(szSecond, TEXT("LocalServer32")) == 0) ||
                        (lstrcmpi(szSecond, TEXT("InprocHandler")) == 0) ||
                        (lstrcmpi(szSecond, TEXT("InprocHandler32")) == 0) ||
                        (lstrcmpi(szSecond, TEXT("InProcServer")) == 0) ||
                        (lstrcmpi(szSecond, TEXT("InProcServer32")) == 0))
                    {
                        cbValue = sizeof(szPath);
                        if (RegQueryValue(hkeySecond, szSecond, szPath,
                            &cbValue) == ERROR_SUCCESS)
                        {
                            (*prlcb)(hdpa, hkeySecond, szSecond, NULL,
                                szPath, pszSource, pszDest);
                        }
                    }
                }

                RegCloseKey(hkeySecond);
            }
        }

        RegCloseKey(hkeyRoot);
    }



    // Now for the typelib section
    if (RegOpenKey(HKEY_CLASSES_ROOT, c_szTypeLib, &hkeyRoot) == ERROR_SUCCESS)
    {
        for (iRootName = 0; RegEnumKey(hkeyRoot, iRootName,
            szRootName, ARRAYSIZE(szRootName)) == ERROR_SUCCESS; iRootName++)
        {
            // Now try to enum this key
            if (RegOpenKey(hkeyRoot, szRootName, &hkeySecond) == ERROR_SUCCESS)
            {
                for (iSecond = 0; RegEnumKey(hkeySecond, iSecond,
                    szSecond, ARRAYSIZE(szSecond)) == ERROR_SUCCESS; iSecond++)
                {
                    if (RegOpenKey(hkeySecond, szSecond, &hkey3) == ERROR_SUCCESS)
                    {
                        cbValue = sizeof(szPath);
                        if (RegQueryValue(hkey3, c_szHelpDir, szPath,
                            &cbValue) == ERROR_SUCCESS)
                        {
                            (*prlcb)(hdpa, hkey3, c_szHelpDir, NULL,
                                szPath, pszSource, pszDest);
                        }

                        for (iThird = 0; RegEnumKey(hkey3, iThird,
                            szSecond, ARRAYSIZE(szSecond)) == ERROR_SUCCESS; iThird++)
                        {
                            HKEY hkey4;
                            // See if there is an interesting value under
                            // this one
                            if (RegOpenKey(hkey3, szSecond, &hkey4) == ERROR_SUCCESS)
                            {
                                cbValue = sizeof(szPath);
                                if (RegQueryValue(hkey4, c_szWin16, szPath,
                                    &cbValue) == ERROR_SUCCESS)
                                {
                                    (*prlcb)(hdpa, hkey4, c_szWin16, NULL,
                                        szPath, pszSource, pszDest);
                                }

                                cbValue = sizeof(szPath);
                                if (RegQueryValue(hkey4, c_szWin32, szPath,
                                    &cbValue) == ERROR_SUCCESS)
                                {
                                    (*prlcb)(hdpa, hkey4, c_szWin32, NULL,
                                        szPath, pszSource, pszDest);
                                }
                                RegCloseKey(hkey4);
                            }
                        }
                        RegCloseKey(hkey3);
                    }
                }

                RegCloseKey(hkeySecond);
            }
        }

        RegCloseKey(hkeyRoot);
    }

    //
    // Now lets go at the root of the class root.
    //
    for (iRootName = 0; RegEnumKey(HKEY_CLASSES_ROOT, iRootName,
        szRootName, ARRAYSIZE(szRootName)) == ERROR_SUCCESS; iRootName++)
    {
        // Now try to enum this key if it is not CLSID
        if ((lstrcmpi(szRootName, TEXT("CLSID")) != 0) &&
            (lstrcmpi(szRootName, c_szTypeLib) != 0) &&
            (RegOpenKey(HKEY_CLASSES_ROOT, szRootName, &hkeySecond) == ERROR_SUCCESS))
        {
            for (iSecond = 0; RegEnumKey(hkeySecond, iSecond,
                szSecond, ARRAYSIZE(szSecond)) == ERROR_SUCCESS; iSecond++)
            {

                static const TCHAR c_szProtocol[] = TEXT("protocol");
                static const TCHAR c_szServer[] = TEXT("Server");
                // Now see if this is one of the keys that we know
                // contain paths of interest.
                if (lstrcmpi(szSecond, TEXT("DefaultIcon")) == 0)
                {
                    cbValue = sizeof(szPath);
                    if (RegQueryValue(hkeySecond, szSecond, szPath,
                        &cbValue) == ERROR_SUCCESS)
                    {
                        (*prlcb)(hdpa, hkeySecond, szSecond, NULL, szPath,
                            pszSource, pszDest);
                    }
                }
                else if ((lstrcmpi(szSecond, c_szProtocol) == 0) ||
                    (lstrcmpi(szSecond, TEXT("Shell")) == 0))
                {
                    // We need to enum the keys under this one and see
                    // if anything interesting.
                    if (RegOpenKey(hkeySecond, szSecond, &hkey3) == ERROR_SUCCESS)
                    {
                        for (iThird = 0; RegEnumKey(hkey3, iThird,
                            szSecond, ARRAYSIZE(szSecond)) == ERROR_SUCCESS; iThird++)
                        {
                            HKEY hkey4;
                            // See if there is an interesting value under
                            // this one
                            if (RegOpenKey(hkey3, szSecond, &hkey4) == ERROR_SUCCESS)
                            {
                                cbValue = sizeof(szPath);
                                if (RegQueryValue(hkey4, c_szServer, szPath,
                                    &cbValue) == ERROR_SUCCESS)
                                {
                                    (*prlcb)(hdpa, hkey4, c_szServer, NULL,
                                        szPath, pszSource, pszDest);
                                }

                                cbValue = sizeof(szPath);
                                if (RegQueryValue(hkey4, TEXT("command"), szPath,
                                    &cbValue) == ERROR_SUCCESS)
                                {
                                    (*prlcb)(hdpa, hkey4, TEXT("command"), NULL,
                                        szPath, pszSource, pszDest);
                                }
                                RegCloseKey(hkey4);
                            }
                        }
                        RegCloseKey(hkey3);
                    }
                }
            }

            RegCloseKey(hkeySecond);
        }
    }

    // First look in the App Paths section
    if (RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\App Paths"), &hkeyRoot) == ERROR_SUCCESS)
    {
        for (iRootName = 0; RegEnumKey(hkeyRoot, iRootName,
            szRootName, ARRAYSIZE(szRootName)) == ERROR_SUCCESS; iRootName++)
        {
            // Now see if the app has a qualifid path here.
            cbValue = sizeof(szPath);
            if (RegQueryValue(hkeyRoot, szRootName, szPath,
                &cbValue) == ERROR_SUCCESS)
            {
                (*prlcb)(hdpa, hkeyRoot, szRootName, NULL,
                    szPath, pszSource, pszDest);
            }

            // Now try to enum this key  for Path Value.
            if (RegOpenKey(hkeyRoot, szRootName, &hkeySecond) == ERROR_SUCCESS)
            {
                DWORD dwType;

                cbValue = sizeof(szPath);

                if (RegQueryValueEx(hkeySecond, (LPTSTR)TEXT("PATH"), NULL, &dwType,
                    (LPBYTE)szPath, &cbValue) == ERROR_SUCCESS)
                {
                    (*prlcb)(hdpa, hkeySecond, NULL, (LPTSTR)TEXT("PATH"),
                        szPath, pszSource, pszDest);
                }

                RegCloseKey(hkeySecond);
            }
        }

        RegCloseKey(hkeyRoot);
    }


    return(TRUE);

}

int CALLBACK _CompareStringsUnimpl(LPVOID sz1, LPVOID sz2, LPARAM lparam)
{
    return(lstrcmpi((LPTSTR)sz1, (LPTSTR)sz2));
}

RLPI    g_rlpi = { NULL, FALSE, FALSE };
BOOL CALLBACK _RLBuildListCallBackUnimpl(HDPA hdpa, HKEY hkey, LPCTSTR pszKey, LPCTSTR pszValueName, LPTSTR pszValue, LPCTSTR pszSource, LPCTSTR pszDest)
{
    LPTSTR pszT;
    int iIndex;

    // BUGBUG:: need to handle ; seperated lists.

    // We have a value, remove any args on it.
    PathRemoveArgsUnimpl(pszValue);

    // Likewise remove everything after a , if there is one.
    // This hits us in things like Icon paths...
    if (NULL != (pszT = StrChr(pszValue, TEXT(','))))
        *pszT = TEXT('\0');

    PathUnquoteSpacesUnimpl(pszValue);

    // Also don't add any relative paths.
    if (PathIsRelativeUnimpl(pszValue) || (lstrlen(pszValue) < 3))
        return(TRUE);

    // Don't try UNC names as this can get expensive...
    if (PathIsUNCUnimpl(pszValue))
        return(TRUE);

    // If it is already in our list, we can simply return now..
    if (DPA_Search(hdpa, pszValue, 0,
        &_CompareStringsUnimpl, 0, DPAS_SORTED) != -1)
        return(TRUE);

    // If it is in our old list then
    if (g_rlpi.hdpaRLList && ((iIndex = DPA_Search(g_rlpi.hdpaRLList, pszValue, 0,
        &_CompareStringsUnimpl, 0, DPAS_SORTED)) != -1))

    {
        // Found the item in the old list.
//
// Too noisy, use your private flag.
//
#ifdef YOUR_PRIVATE_DEBUG_FLAG
        DebugMsg(DM_TRACE, TEXT("rl.bcb: Add from old list %s"), pszValue);
#endif
        DPA_InsertPtr(hdpa,
            DPA_Search(hdpa, pszValue, 0,
                &_CompareStringsUnimpl, 0,
                DPAS_SORTED | DPAS_INSERTBEFORE),
            (LPTSTR)DPA_FastGetPtr(g_rlpi.hdpaRLList, iIndex));
        // now remove it from the old list
        DPA_DeletePtr(g_rlpi.hdpaRLList, iIndex);
    }
    else
    {
        // Not in either list.
        // Now see if we can convert the short name to a long name
        // the simplist way to do this is to create a pidl from it and
        // then convert the pidl back to a path.
        // One to Add!

        TCHAR szLongName[MAX_PATH];
        int cchName;
        LPTSTR psz;

        LPITEMIDLIST pidl = ILCreateFromPath(pszValue);

        if (pidl)
        {
            SHGetPathFromIDList(pidl, szLongName);
            ILFree(pidl);

            if (lstrcmpi(szLongName, pszValue) == 0)
                szLongName[0] = TEXT('\0');   // Don't need both strings.

        }
        else
            szLongName[0] = TEXT('\0');

        cchName = lstrlen(pszValue);

        psz = (LPTSTR)LocalAlloc(LPTR,
            (cchName + 1 + lstrlen(szLongName) + 1) * sizeof(TCHAR));
        if (psz)
        {
            //
            // Too noisy, use your private flag.
            //
#ifdef YOUR_PRIVATE_DEBUG_FLAG
            DebugMsg(DM_TRACE, TEXT("rl.bcb: Add %s"), pszValue);
#endif
            lstrcpy(psz, pszValue);
            lstrcpy(psz + cchName + 1, szLongName);

            return DPA_InsertPtr(hdpa,
                DPA_Search(hdpa, pszValue, 0,
                    &_CompareStringsUnimpl, 0,
                    DPAS_SORTED | DPAS_INSERTBEFORE),
                psz);
        }
    }
    return(TRUE);
}

BOOL WINAPI RLBuildListOfPathsUnimpl()
{
    BOOL fRet = FALSE;
    HDPA hdpa;

    hdpa = DPA_Create(0);
    if (!hdpa)
        goto Error;


    // And initialize the list
    fRet = RLEnumRegistry(hdpa, _RLBuildListCallBackUnimpl, NULL, NULL);


    // If we had on old list destroy it now.

    if (g_rlpi.hdpaRLList)
    {
        // Walk through all of the items in the list and
        // delete all of the strings.
        int i;
        for (i = DPA_GetPtrCount(g_rlpi.hdpaRLList) - 1; i >= 0; i--)
            LocalFree((HLOCAL)DPA_FastGetPtr(g_rlpi.hdpaRLList, i));
        DPA_Destroy(g_rlpi.hdpaRLList);
    }

    g_rlpi.hdpaRLList = hdpa;
    g_rlpi.fListValid = TRUE;     // Say that we are valid...

#ifdef DEBUG
    DebugMsg(DM_TRACE, TEXT("rl.blp time: %ld"), GetCurrentTime() - dwStart);
#endif

Error:

    return(fRet);
}

static int rgiDriveType[26] = {
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1
};

static int rgiHasNetFlags[26] = { 0 };

void WINAPI InvalidateDriveType(int iDrive)
{
    int i;

    if (iDrive < 0)
    {
        //
        //  invalidate all drives
        //
#ifndef DEBUG
        RegDeleteKey(HKEY_CLASSES_ROOT, TEXT("AutoRun.inf"));
#endif

        // Clear these values under the critical section, so any pair will
        // always be in sync

            for (i = 0; i < 26; i++)
            {
                rgiDriveType[i] = -1;
                rgiHasNetFlags[i] = FALSE;
            }

    }
    else if (iDrive < 26)
    {
        TCHAR szDrive[10];
        SHFILEINFO sfi;
        int iIcon = 0;

        //
        //  invalidate a single drive, if the icon for a drive changes
        //  send a notify so links can update..  Handle the case where
        //  the drive was a unavailable net drive...
        //
        if ((rgiDriveType[iDrive] != -1) &&
            ((rgiDriveType[iDrive] & ~DRIVE_TYPE) != ~DRIVE_TYPE) &&
            (((rgiDriveType[iDrive] & DRIVE_TYPE) >= DRIVE_REMOVABLE)
                || (rgiDriveType[iDrive] & DRIVE_NETUNAVAIL)))
        {
            PathBuildRootUnimpl(szDrive, iDrive);
            SHGetFileInfo(szDrive, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX);
            iIcon = sfi.iIcon;
        }

        // Clear these values under the critical section, so they'll always
        // be in sync with each other

        rgiDriveType[iDrive] = -1;
        rgiHasNetFlags[iDrive] = FALSE;


            if (iIcon != 0)
            {
                SHGetFileInfo(szDrive, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX);

                if (iIcon != sfi.iIcon &&
                    (rgiDriveType[iDrive] & ~DRIVE_TYPE) != ~DRIVE_TYPE &&
                    (rgiDriveType[iDrive] & DRIVE_TYPE) >= DRIVE_REMOVABLE)
                {
                    //DebugMsg(DM_TRACE, TEXT("InvalidateDriveType: sending icon change for %s (%d => %d)"), szDrive, iIcon, sfi.iIcon);
                    SHChangeNotify(SHCNE_UPDATEIMAGE, SHCNF_DWORD, (LPCVOID)iIcon, NULL);
                }
            }
    }

    // Invalidate the Drive Name Cache for each of the drives...
    //InvalidateDriveNameCache(iDrive);
}

BOOL Free(PVOID   Buffer)
{
    HANDLE  hMem;

    hMem = *(((PHANDLE)Buffer) - 1);

    LocalUnlock(hMem);

    return(LocalFree(hMem) == NULL);
}

PVOID Alloc(ULONG   Bytes)
{
    HANDLE  hMem;
    PVOID   Buffer;

    hMem = LocalAlloc(LMEM_MOVEABLE, Bytes + sizeof(hMem));

    if (hMem == NULL) {
        return(NULL);
    }

    // Lock down the memory
    //
    Buffer = LocalLock(hMem);
    if (Buffer == NULL) {
        LocalFree(hMem);
        return(NULL);
    }

    //
    // Store the handle at the start of the memory block and return
    // a pointer to just beyond it.
    //

    *((PHANDLE)Buffer) = hMem;

    return (PVOID)(((PHANDLE)Buffer) + 1);
}

void* WINAPI ReAlloc(void* pb, long cb)
{
    if (pb == NULL)
        return Alloc(cb);
    return (void*)LocalReAlloc((HLOCAL)pb, cb, LMEM_ZEROINIT | LMEM_MOVEABLE);

}

void WINAPI ILGlobalFree(LPITEMIDLIST pidl)
{
    if (pidl)
        Free(pidl);
}

LPITEMIDLIST WINAPI ILGlobalClone(LPCITEMIDLIST pidl)
{
    LPITEMIDLIST pidlRet = NULL;

    if (pidl)
    {
        UINT cb = ILGetSize(pidl);

        pidlRet = (LPITEMIDLIST)Alloc(cb);
        if (pidlRet)
        {
            memcpy(pidlRet, pidl, cb);
        }
    }

    return pidlRet;
}
BOOL s_fAbortInvoke = FALSE;

void WINAPI SHAbortInvokeCommand()
{
    s_fAbortInvoke = TRUE;
}



DWORD WINAPI GetProcessDword(DWORD idProcess, LONG iIndex)
{
    //
    // We only ever use - GPD_HINST, GPD_FLAGS (for WPF_WIN16_PROCESS),
    // GPD_EXP_WINVER
    //
    // In IsWin16Process (GPD_FLAGS) we could look at the WOWWORDs via
    // GetWindowLong, or we could look at the GWL_HINST and see if the
    // highword is zero.
    //
    // Window_GetInstance (GPD_HINST) couldn't we just call GetWindowLong
    // with GWL_HINST?
    //
    // ShellExecuteNormal (GPD_HINST) I don't think we can fix this easily.
    // can we get the pei to just have a process id instead?
    //
    // Window_IsLFNAware (GPD_FLAGS & GPD_EXPWINVER) we can get this info from
    // user somehow eh?
    //
    return 0;
}

int CDrives_GetDriveIndex(LPCIDDRIVE pidd)
{
    TCHAR szRoot[4];

    if ((pidd->bFlags & SHID_GROUPMASK) != SHID_COMPUTER)
        return -1;

    szRoot[0] = (TCHAR)(pidd->cName[0]);
    szRoot[1] = TEXT(':');
    szRoot[2] = TEXT('\\');
    szRoot[3] = 0;

    return PathGetDriveNumberUnimpl(szRoot);
}

int RealDriveTypeFlags(int iDrive, BOOL fOKToHitNet)
{
    return 0;
}

const TCHAR c_szRegSetup[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion");

BOOL WINAPI Str_SetPtr(TCHAR FAR* UNALIGNED* ppsz, LPCTSTR psz)
{
    if (!psz)
    {
        if (*ppsz)
        {
            Free((LPTSTR)*ppsz);
            *ppsz = NULL;
        }
    }
    else
    {
        LPTSTR pszNew = (LPTSTR)ReAlloc((LPTSTR)*ppsz, (lstrlen(psz) + 1) * sizeof(TCHAR));
        if (!pszNew)
            return FALSE;

        lstrcpy(pszNew, psz);
        *ppsz = pszNew;
    }
    return TRUE;
}


int WINAPI SHHandleDiskFull(HWND hwnd, int idDrive)
{
    return 0;
}

HMODULE g_hmodOLE = NULL;
STDAPI SHLoadOLE(LPARAM lParam)
{
    HRESULT hres;
    switch (lParam)
    {
    case SHELLNOTIFY_OLELOADED:
        if (!g_hmodOLE)
        {
            // If we are on a low memory machine, we won't load ole in the
            // shell's context since this seems to slow the shell quite a bit
            // in 4 meg.  But if some shell extension or whomever has loaded
            // ole in our context (GetModuleHandle returns non zero), we will
            // init OLE in the shell.
            if (!((GetSystemMetrics(SM_SLOWMACHINE) & 0x0002) &&
                !GetModuleHandle(TEXT("OLE32.DLL"))))
                hres = SHLoadOLE(TRUE);
        }
        break;

    case SHELLNOTIFY_OLEUNLOADED:
        //
        // We never unload OLE from the shell process.
        //
        // hres = _UnloadOLE();
        hres = S_OK;
        break;

    default:
        hres = E_UNEXPECTED;
        break;
    }

    return hres;
}


BOOL WINAPI SHChangeNotifyDeregisterInternal(ULONG ulID)
{
    BOOL fRetval = TRUE;

    return fRetval;
}


static FSNotify s_fsn =
{
        0,
        0,
        NULL,
        NULL,
        NULL,
        1,
};


ULONG WINAPI SHChangeNotifyRegisterInternal(HWND hwnd, int fSources, LONG fEvents, UINT wMsg, int cEntries, void* pfsne)
{
    return 0;
}

BOOL WINAPI SHChangeRegistrationReceive(HANDLE hChangeRegistration, DWORD dwProcId)
{
    LPSHChangeRegistration  pshcr;
    BOOL fResult = FALSE;

    pshcr = SHLockShared(hChangeRegistration, dwProcId);
    if (pshcr)
    {
        switch (pshcr->uCmd)
        {
        case SHCR_CMD_REGISTER:
        {
            SHChangeNotifyEntry fsne;

            fsne.pidl = NULL;
            fsne.fRecursive = pshcr->fRecursive;
            if (pshcr->uidlRegister)
                fsne.pidl = (LPITEMIDLIST)((LPBYTE)pshcr + pshcr->uidlRegister);

            pshcr->ulID = SHChangeNotifyRegisterInternal(
                pshcr->hwnd, pshcr->fSources,
                pshcr->lEvents, pshcr->uMsg,
                1, &fsne);
            fResult = TRUE;
        }
        break;
        case SHCR_CMD_DEREGISTER:
            fResult = SHChangeNotifyDeregisterInternal(pshcr->ulID);
            break;
        default:
            break;
        }
        SHUnlockShared(pshcr);
    }
    return fResult;
}


// Add data to an MRU list.
typedef int (cdecl FAR* MRUCMPDATAPROC)(const void FAR*, const void FAR*,
    size_t);

int WINAPI AddMRUData(HANDLE hMRU, const void FAR* lpData, UINT cbData)
{
    TCHAR cFirst;
    int iSlot = -1;
    LPTSTR lpTemp;
    LPBYTE FAR* ppData;
    int i;
    UINT uMax;
    MRUCMPDATAPROC lpfnCompare;
    BOOL fShouldWrite = !(pMRU->fFlags & MRU_CACHEWRITE);

#ifdef DEBUG
    DWORD dwStart = GetTickCount();
#endif
    if (hMRU == NULL)
        return(-1);     // Error

    uMax = pMRU->uMax;
    lpfnCompare = (MRUCMPDATAPROC)pMRU->lpfnCompare;

    // Check if the data already exists in the list.
    for (i = 0, ppData = &NTHDATA(pMRU, 0); (UINT)i < uMax; ++i, ++ppData)
    {
        if (*ppData && (DATASIZE(*ppData) == cbData)
            && ((*lpfnCompare)(lpData, DATAPDATA(*ppData), cbData) == 0))
        {
            // found it, so don't do the write out
            cFirst = i + BASE_CHAR;
            iSlot = i;
            goto FoundEntry;
        }
    }

    // Attempt to find an unused entry.  Count up the used entries at the
    // same time.
    for (i = 0, ppData = &NTHDATA(pMRU, 0); ; ++i, ++ppData)
    {
        if ((UINT)i >= uMax)
            // If we got to the end of the list.
        {
            // use the entry at the end of the cOrder list
            cFirst = pMRU->cOrder[uMax - 1];
            ppData = &NTHDATA(pMRU, cFirst - BASE_CHAR);
            break;
        }

        if (!*ppData)
            // If the entry is not used.
        {
            cFirst = i + BASE_CHAR;
            break;
        }
    }

    *ppData = ReAlloc(*ppData, cbData + sizeof(DWORD));
    if (*ppData)
    {
        TCHAR szTemp[2];

        *((LPDWORD)(*ppData)) = cbData;
        memcpy(DATAPDATA(*ppData), lpData, cbData);

        iSlot = (int)(cFirst - BASE_CHAR);

        szTemp[0] = cFirst;
        szTemp[1] = TEXT('\0');

        RegSetValueEx(pMRU->hKey, szTemp, 0L, REG_BINARY, (LPVOID)lpData, cbData);
        fShouldWrite = TRUE;
    }
    else
    {
        // Since iSlot == -1, we will remove the reference to cFirst
        // below.
    }

FoundEntry:
    // Remove any previous reference to cFirst.
    lpTemp = StrChr(pMRU->cOrder, cFirst);
    if (lpTemp)
    {
        lstrcpy(lpTemp, lpTemp + 1);
    }

    if (iSlot != -1)
    {
        // shift everything over and put cFirst at the front
        memcpy(pMRU->cOrder + 1, pMRU->cOrder, pMRU->uMax * sizeof(TCHAR));
        pMRU->cOrder[0] = cFirst;
    }

    if (fShouldWrite) {
        RegSetValueEx(pMRU->hKey,
            TEXT("MRUList"),
            0L,
            REG_SZ,
            (CONST BYTE*)pMRU->cOrder,
            sizeof(TCHAR) * (lstrlen(pMRU->cOrder) + 1));
        pMRU->fFlags &= ~MRU_ORDERDIRTY;
    }
    else
        pMRU->fFlags |= MRU_ORDERDIRTY;

#ifdef DEBUG
    // DebugMsg(DM_TRACE, TEXT("AddMRU: %d msec"), LOWORD(GetTickCount()-dwStart));
#endif
    return(iSlot);
}

HANDLE WINAPI CreateMRUList(LPMRUINFO lpmi)
{
    return;
}

__inline BOOL DBL_BSLASH(LPCTSTR psz)
{
    return (psz[0] == TEXT('\\') && psz[1] == TEXT('\\'));
}

int FAR CDECL _mymemcmp(const void FAR* pBuf1, const void FAR* pBuf2, size_t cb)
{
    UINT i;
    const BYTE FAR* lpb1, FAR* lpb2;

    //Assert(pBuf1);
    //Assert(pBuf2);

    lpb1 = pBuf1; lpb2 = pBuf2;

    for (i = 0; i < cb; i++)
    {
        if (*lpb1 > *lpb2)
            return 1;
        else if (*lpb1 < *lpb2)
            return -1;

        lpb1++;
        lpb2++;
    }

    return 0;
}

int WINAPI FindMRUData(HANDLE hMRU, const void FAR* lpData, UINT cbData, LPINT lpiSlot)
{
    TCHAR cFirst;
    int iSlot = -1;
    LPTSTR lpTemp;
    LPBYTE FAR* ppData;
    int i;
    UINT uMax;
    MRUCMPDATAPROC lpfnCompare;

#ifdef DEBUG
    DWORD dwStart = GetTickCount();
#endif

    if (hMRU == NULL)
        return(-1); // Error state.

    uMax = pMRU->uMax;
    lpfnCompare = pMRU->lpfnCompare;

    /* Find the item in the list.
     */
    for (i = 0, ppData = &NTHDATA(pMRU, 0); (UINT)i < uMax; ++i, ++ppData)
    {
        int cbUseSize;
        if (!*ppData)
            continue;

        // if there's something other than a mem compare,
        // don't require the sizes to be equal in order for the
        // data to be equivalent.

        if (pMRU->lpfnCompare == _mymemcmp) {
            if (DATASIZE(*ppData) != cbData)
                continue;

            cbUseSize = cbData;
        }
        else {

            cbUseSize = min(DATASIZE(*ppData), cbData);
        }

        if ((*lpfnCompare)(lpData, DATAPDATA(*ppData), cbUseSize) == 0)
        {
            // So i now has the slot number in it.
            if (lpiSlot != NULL)
                *lpiSlot = i;

            // Now convert the slot number into an index number
            cFirst = i + BASE_CHAR;
            lpTemp = StrChr(pMRU->cOrder, cFirst);
            //Assert(lpTemp);
            return((lpTemp == NULL) ? -1 : (int)(lpTemp - (LPTSTR)pMRU->cOrder));
        }
    }

    return -1;
}

void FSNRemoveInterruptEvent(LPCITEMIDLIST pidl)
{
    LPFSIntEvent lpfsie;
    int i, j;

    if (!s_fsn.hdpaIntEvents)
        return;

#ifdef FSNIDEBUG
    {
        TCHAR szPath[MAX_PATH];
        SHGetPathFromIDList(pidl, szPath);
        DebugMsg(DM_TRACE, TEXT("FSNOTIFY: Remove: %s"), szPath);
    }
#endif



    // assumes caller has checked that sp_fsn.hdsaIntEvents exists
    // and has already grabbed the semaphore

    for (j = DPA_GetPtrCount(s_fsn.hdpaIntEvents) - 1; j >= 0; j--) {
        HDSA hdsaIntEvents;

        hdsaIntEvents = DPA_FastGetPtr(s_fsn.hdpaIntEvents, j);

        for (i = DSA_GetItemCount(hdsaIntEvents) - 1; i >= 0; i--) {
            lpfsie = DSA_GetItemPtr(hdsaIntEvents, i);

#ifdef FSNIDEBUG
            {
                TCHAR szPath[MAX_PATH];
                SHGetPathFromIDList(lpfsie->pidl, szPath);
                DebugMsg(DM_TRACE, TEXT("FSNOTIFY: comparing against: %s"), szPath);
            }
#endif
            // check for immediate parent.  no recursive FindFirstChangeNotify allowed.
            if (ILIsParent(lpfsie->pidl, pidl, TRUE)
                || ILIsEqual(lpfsie->pidl, pidl)) {

#ifdef FSNIDEBUG
                    {
                        TCHAR szPath[MAX_PATH];
                        DebugMsg(DM_TRACE, TEXT("FSNOTIFY: RemoveEvent found: %x"), lpfsie);
                        SHGetPathFromIDList(lpfsie->pidl, szPath);
                        DebugMsg(DM_TRACE, TEXT("FSNOTIFY: removing: %s %d"), szPath, lpfsie->iCount);
                    }
#endif
                    lpfsie->iCount--;

                    if (lpfsie->iCount == 0) {

                        ILGlobalFree((LPITEMIDLIST)lpfsie->pidl);
                        DSA_DeleteItem(hdsaIntEvents, i);
                    }
            }
        }
    }

}

LPCTSTR GetPCEnd(LPCTSTR lpszStart)
{
    LPCTSTR lpszEnd;

    lpszEnd = StrChr(lpszStart, TEXT('\\'));
    if (!lpszEnd)
    {
        lpszEnd = lpszStart + lstrlen(lpszStart);
    }

    return lpszEnd;
}

BOOL ChrCmpW(WORD w1, WORD wMatch)
{
#ifdef UNICODE
    return(!(w1 == wMatch));
#else
    SetLastErrorEx(ERROR_CALL_NOT_IMPLEMENTED, SLE_WARNING);
    return FALSE;
#endif
}

LPWSTR FAR PASCAL StrRChrW(LPCWSTR lpStart, LPCWSTR lpEnd, WORD wMatch)
{
#ifdef UNICODE

    LPCWSTR lpFound = NULL;

    if (!lpEnd)
        lpEnd = lpStart + lstrlenW(lpStart);

    for (; lpStart < lpEnd; lpStart++)
    {
        if (!ChrCmpW(*(UNALIGNED WORD FAR*)lpStart, wMatch))
            lpFound = lpStart;
    }
    return ((LPWSTR)lpFound);

#else

    SetLastErrorEx(ERROR_CALL_NOT_IMPLEMENTED, SLE_WARNING);
    return NULL;

#endif
}

LPCTSTR PCStart(LPCTSTR lpszStart, LPCTSTR lpszEnd)
{
    LPCTSTR lpszBegin = StrRChrW(lpszStart, lpszEnd, TEXT('\\'));
    if (!lpszBegin)
    {
        lpszBegin = lpszStart;
    }
    return lpszBegin;
}

typedef
BOOL
(WINAPI* PFNISPATHSHARED)(
    IN LPCTSTR lpPath,
    IN BOOL fRefresh
    );
BOOL      s_fShareLoaded = FALSE;
HINSTANCE s_hmodShare = NULL;
PFNISPATHSHARED g_pfnIsPathShared = NULL;
BOOL ShareDLL_Init(void)
{
    TCHAR szPath[MAX_PATH];
    LONG cb;

    //BUGBUG!!! what if share.dll is gone?

    // See if we have already tried to load this in this context
    if (s_fShareLoaded)
        return(TRUE);

    s_fShareLoaded = TRUE;
    szPath[0] = 0;
    cb = sizeof(szPath);
    RegQueryValue(HKEY_CLASSES_ROOT, TEXT("Network\\SharingHandler"), szPath, &cb);
    if (szPath[0]) {
        s_hmodShare = LoadLibrary(szPath);
        if (ISVALIDHINSTANCE(s_hmodShare)) {
            g_pfnIsPathShared = (PFNISPATHSHARED)GetProcAddress(s_hmodShare, TEXT("Network\\SharingHandler"));
        }
    }

    return TRUE;
}




BOOL IsShared(LPCTSTR pszPath, BOOL fUpdateCache)
{
    ShareDLL_Init();

#ifdef ALIGNMENT_SCENARIO
    {
        TCHAR szTmp[MAX_PATH];
        ualstrcpyn(szTmp, pszPath, ARRAYSIZE(szTmp));

        if (g_pfnIsPathShared)
        {
            return g_pfnIsPathShared(szTmp, fUpdateCache);
        }
    }
#else
    {
        if (g_pfnIsPathShared) {
            return g_pfnIsPathShared(pszPath, fUpdateCache);
        }
    }
#endif

    return FALSE;
}

BOOL IsILShared(LPCITEMIDLIST pidl, BOOL fUpdateCache)
{
    TCHAR szTemp[MAX_PATH];
    SHGetPathFromIDList(pidl, szTemp);
    return IsShared(szTemp, fUpdateCache);
}

LPITEMIDLIST WINAPI _ILCreate(UINT cbSize)
{
    LPITEMIDLIST pidl = (LPITEMIDLIST)SHAlloc(cbSize);
    if (pidl)
        memset(pidl, 0, cbSize);      // zero-init for external task allocator

    return pidl;
}

//HRESULT WINAPI ILLoadFromStreamEx(LPSTREAM pstm, LPITEMIDLIST* ppidl)
//{
//    HRESULT hres;
//    ULONG cb;
//
//
//    // Delete the old one if any.
//    if (*ppidl)
//    {
//        ILFree(*ppidl);
//        *ppidl = NULL;
//    }
//
//    // Read the size of the IDLIST
//    cb = 0;             // WARNING: We need to fill its HIWORD!
//    hres = pstm->lpVtbl->Read(pstm, &cb, sizeof(USHORT), NULL); // Yes, USHORT
//    if (SUCCEEDED(hres) && cb)
//    {
//        // Create a IDLIST
//        LPITEMIDLIST pidl = _ILCreate(cb);
//        if (pidl)
//        {
//            // Read its contents
//            hres = pstm->lpVtbl->Read(pstm, pidl, cb, NULL);
//            if (SUCCEEDED(hres))
//            {
//#define SUPPORT_M6PIDL
//#ifdef SUPPORT_M6PIDL
//                if (pidl->mkid.cb == 3 && (pidl->mkid.abID[0] == 0x11 || pidl->mkid.abID[0] == 0x12))
//                {
//                    LPITEMIDLIST pidlHack = ILCombine((LPCITEMIDLIST)(pidl->mkid.abID[0] == 0x11 ? &c_idlDrives : &c_idlNet), _ILNext(pidl));
//                    ILFree(pidl);
//                    pidl = pidlHack;
//                }
//#endif // SUPPORT_M6PIDL
//                * ppidl = pidl;
//            }
//            else
//            {
//                ILFree(pidl);
//            }
//        }
//        else
//        {
//            hres = ResultFromScode(E_OUTOFMEMORY);
//        }
//    }
//
//    return hres;
//}

BOOL g_fUseOle = TRUE;  // enable OLE drag&drop by default!
STDAPI SHFlushClipboard(void)
{
    HRESULT hres = NOERROR;
    if (g_hmodOLE && g_fUseOle)
    {
        LPDATAOBJECT pdtobj = 0;
        //hres = g_pfnOleGetClipboard(&pdtobj);
        if (SUCCEEDED(hres))
        {
            //hres = g_pfnOleFlushClipboard();
            pdtobj->lpVtbl->Release(pdtobj);
        }
    }
    return 0;
}

int WINAPI StrToOleStr(LPOLESTR pwsz, LPCTSTR psz)
{
#ifdef UNICODE
    LPCTSTR pszOriginal = psz;
    TCHAR ch;

    while (ch = TEXT('\0') != (*pwsz++ = *psz++), ch)
        NULL;

    return(psz - pszOriginal);
#else
    return MultiByteToWideChar(CP_ACP, 0, psz, -1, pwsz, MAX_PATH);
#endif
}

int WINAPI OleStrToStrN(LPTSTR psz, int cchMultiByte, LPCOLESTR pwsz, int cchWideChar)
{
#ifdef UNICODE
    int cchOutput;
    VDATEINPUTBUF(psz, OLECHAR, cchMultiByte);

    if ((cchMultiByte > cchWideChar) && (cchWideChar != -1))
    {
        cchMultiByte = cchWideChar;
    }
    cchOutput = cchMultiByte;

    // HACK: Ignore DBCS for now
    while (cchMultiByte)
    {
        if ((*psz++ = (TCHAR)*pwsz++) == TEXT('\0'))
            return(cchOutput - cchMultiByte + 1);
        cchMultiByte--;
    };
    if (cchWideChar == -1)
        --psz;

    *psz = TEXT('\0');
    return cchOutput;
#else
    VDATEINPUTBUF(psz, CHAR, cchMultiByte);
    return WideCharToMultiByte(CP_ACP, 0, pwsz, cchWideChar, psz, cchMultiByte, NULL, NULL);
#endif // UNICODE
}

BOOL WINAPI ParseField(LPCTSTR szData, int n, LPTSTR szBuf, int iBufLen)
{
    BOOL  fQuote = FALSE;
    LPCTSTR pszInf = szData;
    LPTSTR ptr;
    int   iLen = 1;

    if (!szData || !szBuf)
        return FALSE;

    /*
    * find the first separator
    */
    while (*pszInf && !ISSEP(*pszInf))
    {
        if (*pszInf == QUOTE)
            fQuote = !fQuote;
        pszInf = CharNext(pszInf);
    }

    if (n == 0 && *pszInf != TEXT('='))
        return FALSE;

    if (n > 0 && *pszInf == TEXT('=') && !fQuote)
        // Change szData to point to first field
        szData = ++pszInf; // Ok for DBCS

    /*
    *   locate the nth comma, that is not inside of quotes
    */
    fQuote = FALSE;
    while (n > 1)
    {
        while (*szData)
        {
            if (!fQuote && ISSEP(*szData))
                break;

            if (*szData == QUOTE)
                fQuote = !fQuote;

            szData = CharNext(szData);
        }

        if (!*szData)
        {
            szBuf[0] = 0;      // make szBuf empty
            return FALSE;
        }

        szData = CharNext(szData); // we could do ++ here since we got here
        // after finding comma or equal
        n--;
    }

    /*
    * now copy the field to szBuf
    */
    while (ISWHITE(*szData))
        szData = CharNext(szData); // we could do ++ here since white space can
    // NOT be a lead byte
    fQuote = FALSE;
    ptr = szBuf;      // fill output buffer with this
    while (*szData)
    {
        if (*szData == QUOTE)
        {
            //
            // If we're in quotes already, maybe this
            // is a double quote as in: "He said ""Hello"" to me"
            //
            if (fQuote && *(szData + 1) == QUOTE)    // Yep, double-quoting - QUOTE is non-DBCS
            {
                if (iLen < iBufLen)
                {
                    *ptr++ = QUOTE;
                    ++iLen;
                }
                szData++;                   // now skip past 1st quote
            }
            else
                fQuote = !fQuote;
        }
        else if (!fQuote && ISSEP(*szData))
            break;
        else
        {
            if (iLen < iBufLen)
            {
                *ptr++ = *szData;                  // Thank you, Dave
                ++iLen;
            }

            if (IsDBCSLeadByte(*szData) && (iLen < iBufLen))
            {
                *ptr++ = szData[1];
                ++iLen;
            }
        }
        szData = CharNext(szData);
    }
    /*
    * remove trailing spaces
    */
    while (ptr > szBuf)
    {
        ptr = CharPrev(szBuf, ptr);
        if (!ISWHITE(*ptr))
        {
            ptr = CharNext(ptr);
            break;
        }
    }
    *ptr = 0;
    return TRUE;
}



DWORD RefreshDaylightInformation(BOOL fChangeTime)
{
    return 0;
}

void WINAPI SHFreeUnusedLibraries()
{
    return 0;
}

DWORD WINAPI SHWaitForFileToOpen(LPCITEMIDLIST pidl, UINT uOptions, DWORD dwTimeout)
{
    HWND    hwndShell;
    HANDLE  hWaitOp;
    HANDLE  hEvent;
    DWORD   dwProcIdSrc;
    DWORD   dwReturn = WAIT_OBJECT_0; // we need a default

    return dwReturn;
}

void WINAPI SetAppStartingCursor(HWND hwnd, BOOL bSet)
{
#ifdef WINNT
    DWORD dwTargetProcID;
#endif
    //g_hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
    if (hwnd && IsWindow(hwnd)) {
        HWND hwndOwner;
        while ((NULL != (hwndOwner = GetParent(hwnd))) || (NULL != (hwndOwner = GetWindow(hwnd, GW_OWNER)))) {
            hwnd = hwndOwner;
        }

#ifdef WINNT
        // SendNotify is documented to only work in-process (and can
        // crash if we pass the pnmhdr across process boundaries on
        // NT, because DLLs aren't all shared in one address space).
        // So, if this SendNotify would go cross-process, blow it off.

        GetWindowThreadProcessId(hwnd, &dwTargetProcID);

        if (GetCurrentProcessId() == dwTargetProcID)
#endif
            //SendNotify(hwnd, NULL, bSet ? NM_STARTWAIT : NM_ENDWAIT, NULL);
    }
}



void WINAPI ReceiveAddToRecentDocs(HANDLE hARD, DWORD dwProcId)
{
    PXMITARD        px;
    LPITEMIDLIST    pidl = NULL;
    LPTSTR          pszPath = NULL;

    px = SHLockShared(hARD, dwProcId);
    if (!px)
        return;

    if (px->dwOffsetPath)
        pszPath = (LPTSTR)((LPBYTE)px + px->dwOffsetPath);
    if (px->dwOffsetPidl)
        pidl = (LPITEMIDLIST)((LPBYTE)px + px->dwOffsetPidl);

    //InternalAddToRecentDocs(pidl, pszPath);

    SHUnlockShared(px);
    SHFreeShared(hARD, dwProcId);
}

HRESULT _GetILIndexGivenPXIcon(LPEXTRACTICON pxicon, UINT uFlags, LPCITEMIDLIST pidl, int* piImage, BOOL fAnsiCrossOver)
{
    TCHAR szIconFile[MAX_PATH];
#ifdef UNICODE
    CHAR szIconFileA[MAX_PATH];
    IExtractIconA* pxiconA = (IExtractIconA*)pxicon;
#endif
    int iIndex;
    int iImage = -1;
    UINT wFlags = 0;
    HRESULT hres = 0;

    return hres;
}

HRESULT SHGetIconFromPIDL(IShellFolder* psf, IShellIcon* psi, LPCITEMIDLIST pidl, UINT flags, int* piImage)
{
    IExtractIcon* pxi;
    HRESULT hres;

    if (psi)
    {
        hres = psi->lpVtbl->GetIconOf(psi, pidl, flags, piImage);
    }

    *piImage = Shell_GetCachedImageIndex(TEXT("shell32.dll"), II_DOCNOASSOC, 0);

    hres = psf->lpVtbl->GetUIObjectOf(psf,
        NULL, 1, pidl ? &pidl : NULL, &IID_IExtractIcon, NULL, &pxi);

    if (SUCCEEDED(hres))
    {
        hres = _GetILIndexGivenPXIcon(pxi, flags, pidl, piImage, FALSE);
        //pxi->lpVtbl->Release(pxi);
    }
#ifdef UNICODE
    else
    {
        //
        // Try the ANSI interface, see if we are dealing with an old set of code
        //
        IExtractIconA* pxiA;

        hres = psf->lpVtbl->GetUIObjectOf(psf,
            NULL, 1, pidl ? &pidl : NULL, &IID_IExtractIconA, NULL, &pxiA);
        if (SUCCEEDED(hres))
        {
            hres = _GetILIndexGivenPXIcon(
                (IExtractIcon*)pxiA, // cast to relieve grief
                flags, pidl, piImage, TRUE); // indicate Ansi ver.
            //pxiA->lpVtbl->Release(pxiA);
        }
    }
#endif

    return hres;
}






















//
// Function loader
//
#define MODULE_VARNAME(NAME) hMod_ ## NAME

#define LOAD_MODULE(NAME)                                        \
HMODULE MODULE_VARNAME(NAME) = LoadLibraryW(L#NAME ".dll");      \
if (!MODULE_VARNAME(NAME))                                       \
    return TRUE;

#define LOAD_FUNCTION(MODULE, FUNCTION)                                      \
*(FARPROC *)&FUNCTION = GetProcAddress(MODULE_VARNAME(MODULE), #FUNCTION);   \
if (!FUNCTION)                                                               \
	return FALSE;

#define LOAD_ORDINAL(MODULE, FUNCNAME, ORDINAL)                                   \
*(FARPROC *)&FUNCNAME = GetProcAddress(MODULE_VARNAME(MODULE), (LPCSTR)ORDINAL);  \
if (!FUNCNAME)                                                                    \
	return TRUE;


BOOL SHUndocInit(void)
{

    LOAD_MODULE(shell32);
    LOAD_ORDINAL(shell32, SHFindComputer, 91);
    LOAD_ORDINAL(shell32, RegisterShellHook, 181);
    LOAD_ORDINAL(shell32, RunFileDlg, 61);
    LOAD_ORDINAL(shell32, ExitWindowsDialog, 60);
    LOAD_ORDINAL(shell32, FileIconInit, 660);
    LOAD_ORDINAL(shell32, ILGetDisplayName, 15);

    LOAD_MODULE(user32);
    LOAD_FUNCTION(user32, DrawCaptionTempW);
    //LOAD_ORDINAL(user32, SetShellWindow,2369);
    LOAD_FUNCTION(user32, SetShellWindow);

    return TRUE;
}


