#ifndef _FSMENU_H
#define _FSMENU_H

#include <shlobj.h>
#include "..\Headers\cabinet.h"
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

typedef enum
{
        FMF_NONE                = 0x0000,
        FMF_NOEMPTYITEM         = 0x0001,
        FMF_INCLUDEFOLDERS      = 0x0002,
        FMF_NOPROGRAMS          = 0x0004,
        FMF_FILESMASK           = 0x0007,
        FMF_LARGEICONS          = 0x0008,
        FMF_NOBREAK             = 0x0010,
        FMF_NOABORT             = 0x0020,
} FMFLAGS;

#define FMAI_SEPARATOR          0x00000001

typedef void (CALLBACK *PFNFMCALLBACK)(LPITEMIDLIST, LPITEMIDLIST);

UINT        WINAPI FileMenu_ReplaceUsingPidl(HMENU hmenu, UINT idNewItems,  LPITEMIDLIST pidl, UINT fMenuFilter, PFNFMCALLBACK pfncb);
BOOL        WINAPI FileMenu_InitMenuPopup(HMENU hmenu);
LRESULT WINAPI FileMenu_DrawItem(HWND hwnd, DRAWITEMSTRUCT FAR *lpdi);
LRESULT WINAPI FileMenu_MeasureItem(HWND hwnd, MEASUREITEMSTRUCT FAR *lpmi);
UINT        WINAPI FileMenu_DeleteAllItems(HMENU hmenu);
 LRESULT WINAPI FileMenu_HandleMenuChar(HMENU hmenu, TCHAR ch);
 BOOL        WINAPI FileMenu_GetLastSelectedItemPidls(HMENU hmenu, LPITEMIDLIST *ppidlFolder, LPITEMIDLIST *ppidlItem);
 HMENU       WINAPI FileMenu_FindSubMenuByPidl(HMENU hmenu, LPITEMIDLIST pidl);
 UINT        WINAPI FileMenu_InsertUsingPidl(HMENU hmenu, UINT idNewItems,  LPITEMIDLIST pidl, FMFLAGS fmf, UINT fMenuFilter, PFNFMCALLBACK pfncb);
 void        WINAPI FileMenu_Invalidate(HMENU hmenu);
 HMENU   WINAPI FileMenu_Create(COLORREF clr, int cxBmpGap, HBITMAP hbmp, int cySel, FMFLAGS fmf);
 BOOL    WINAPI FileMenu_AppendItem(HMENU hmenu, LPTSTR psz, UINT id, int iImage, HMENU hmenuSub, UINT cyItem);
 BOOL    WINAPI FileMenu_TrackPopupMenuEx(HMENU hmenu, UINT Flags, int x, int y, HWND hwndOwner, LPTPMPARAMS lpTpm);
 BOOL        WINAPI FileMenu_DeleteItemByCmd(HMENU hmenu, UINT id);
 void        WINAPI FileMenu_Destroy(HMENU hmenu);
 BOOL        WINAPI FileMenu_EnableItemByCmd(HMENU hmenu, UINT id, BOOL fEnable);
 BOOL        WINAPI FileMenu_DeleteSeparator(HMENU hmenu);
 BOOL        WINAPI FileMenu_DeleteMenuItemByFirstID(HMENU hmenu, UINT id);
 DWORD       WINAPI FileMenu_GetItemExtent(HMENU hmenu, UINT iItem);
 BOOL        WINAPI FileMenu_DeleteItemByIndex(HMENU hmenu, UINT iItem);
 void        WINAPI FileMenu_AbortInitMenu(void);
 UINT        WINAPI FileMenu_AppendFilesForPidl(HMENU hmenu, LPITEMIDLIST pidl, BOOL bInsertSeparator);
 UINT        WINAPI FileMenu_AddFilesForPidl(HMENU hmenu, UINT iPos, UINT idNewItems,
                                                        LPITEMIDLIST pidl, FMFLAGS fmf, UINT fMenuFilter, PFNFMCALLBACK pfncb);
#endif //_FSMENU_H
