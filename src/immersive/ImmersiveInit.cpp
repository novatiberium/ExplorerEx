#include "ImmersiveCommon.h"
#include "ImmersiveShell.h"
#include "ImmersiveFactory.h"
#include "hookapi.h"

bool s_EnableImmersiveShellStack = true;

extern "C" void * _ReturnAddress(void);
#pragma intrinsic(_ReturnAddress)

typedef HWND(WINAPI* CreateWindowInBandAPI)(DWORD, LPWSTR, PVOID, PVOID, PVOID, PVOID, PVOID, PVOID, PVOID, PVOID, PVOID, PVOID, DWORD);
static CreateWindowInBandAPI CreateWindowInBandOrig;

typedef HWND(WINAPI* CreateWindowInBandExAPI)(DWORD, LPWSTR, PVOID, PVOID, PVOID, PVOID, PVOID, PVOID, PVOID, PVOID, PVOID, PVOID, DWORD, DWORD);
static CreateWindowInBandExAPI CreateWindowInBandExOrig;

typedef BOOL(WINAPI* GetWindowBandAPI)(HWND, DWORD*);
static GetWindowBandAPI GetWindowBandOrig;

typedef HWND(WINAPI* SetWindowBandApi)(HWND hwnd, HWND hwndInsertAfter, DWORD dwBand);
static SetWindowBandApi SetWindowBandApiOrg;

typedef BOOL(WINAPI* RegisterHotKeyApi)(HWND hwnd, int id, UINT fsMod, UINT vk);
static RegisterHotKeyApi RegisterHotKeyApiOrg;

// Ittr: The following 3 functions are here rather than any specific imports header because they are used by 2 different patch types
HWND WINAPI CreateWindowInBandNew(DWORD dwExStyle,
	LPCWSTR lpClassName,
	LPCWSTR lpWindowName,
	DWORD dwStyle,
	int x,
	int y,
	int nWidth,
	int nHeight,
	HWND hwndParent,
	HMENU hMenu,
	HINSTANCE hInstance,
	LPVOID lpParam,
	DWORD dwBand)
{
	if (s_EnableImmersiveShellStack == 1) // immersive enabled
	{
		DWORD p0 = (DWORD)_ReturnAddress();
		dwExStyle = dwExStyle | WS_EX_TOOLWINDOW; // TODO is this needed?
		HWND ret = CreateWindowExW(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hwndParent, hMenu, hInstance, lpParam);

		// We do this to eliminate the ghost window
		BOOL shouldCloak = true;
		WCHAR titleBuffer[MAX_PATH];
		GetClassName(ret, titleBuffer, sizeof(titleBuffer));
		WCHAR afwTitle[23] = L"ApplicationFrameWindow";
		if (strcmp((char*)titleBuffer, (char*)afwTitle) == 0)
			DwmSetWindowAttribute(ret, DWMWA_CLOAK, &shouldCloak, sizeof(shouldCloak));

		dbgprintf(L"CREATEWINDOWINBANDNEW %i", dwBand);

		if (ret)
		{
			SetProp(ret, L"UIA_WindowVisibilityOverriden", (HANDLE)2);
			SetProp(ret, L"explorer7.WindowBand", (HANDLE)dwBand);
		}

		return ret;
	}
	else // Preserve legacy codepath for Windows 8.1 and non-immersive users
	{
		DWORD p0 = (DWORD)_ReturnAddress();
		dwStyle = dwStyle | WS_EX_TOOLWINDOW;
		HWND ret = CreateWindowInBandOrig(dwExStyle, (LPWSTR)lpClassName, (PVOID)lpWindowName, (PVOID)dwStyle, (PVOID)x, (PVOID)y, (PVOID)nWidth, (PVOID)nHeight, hwndParent, hMenu, hInstance, lpParam, dwBand & 1);
		dbgprintf(L"%p: CreateWindowInBand %p %s %p %p %p %p %p %p %p %p %p %p %p = %p %p", p0, dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hwndParent, hMenu, hInstance, lpParam, dwBand, ret, GetLastError());
		SetProp(ret, L"explorer7.WindowBand", (HANDLE)dwBand);
		return ret;
	}
}

HWND WINAPI CreateWindowInBandExNew(DWORD exStyle, LPWSTR szClassName, PVOID p3, PVOID p4, PVOID p5, PVOID p6, PVOID p7, PVOID p8, PVOID p9, PVOID p10, PVOID p11, PVOID p12, DWORD p13, DWORD dwTypeFlags)
{
	DWORD p0 = (DWORD)_ReturnAddress();
	exStyle = exStyle | WS_EX_TOOLWINDOW;
	HWND ret = CreateWindowInBandExOrig(exStyle, szClassName, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13 & 1, dwTypeFlags);

	// We do this to eliminate the ghost window
	BOOL shouldCloak = true;
	WCHAR titleBuffer[MAX_PATH];
	GetClassName(ret, titleBuffer, sizeof(titleBuffer));
	WCHAR afwTitle[23] = L"ApplicationFrameWindow";
	if (strcmp((char*)titleBuffer, (char*)afwTitle) == 0)
		DwmSetWindowAttribute(ret, DWMWA_CLOAK, &shouldCloak, sizeof(shouldCloak));

	dbgprintf(L"%p: CreateWindowInBandEx %p %s %p %p %p %p %p %p %p %p %p %p %p = %p %p", p0, exStyle, szClassName, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, ret, GetLastError());
	dbgprintf(L"CreateWindowInBandExOrig %i", p13);

	SetProp(ret, L"UIA_WindowVisibilityOverriden", (HANDLE)2);
	SetProp(ret, L"explorer7.WindowBand", (HANDLE)p13);
	return ret;
}

BOOL WINAPI SetWindowBandNew(HWND hwnd, HWND hwndInsertAfter, DWORD flags)
{
	SetProp(hwnd, L"explorer7.WindowBand", (HANDLE)flags);
	dbgprintf(L"SetWindowBandNew %i", flags);
	return TRUE;
}

BOOL WINAPI RegisterWindowHotkeyNew(HWND hwnd, int id, UINT mod, UINT vk)
{
	BOOL res = RegisterHotKeyApiOrg(hwnd, id, mod, vk);

	if (!res)
	{
		return TRUE;
	}

	return TRUE;
}

BOOL WINAPI GetWindowBandNew(HWND hwnd, DWORD* out)
{
	BOOL ret = GetWindowBandOrig(hwnd, out);
	DWORD origband = (DWORD)GetProp(GetAncestor(hwnd, GA_ROOTOWNER), L"explorer7.WindowBand");
	//dbgprintf(L"GetWindowBand %p %p %p",hwnd,*out,origband);
	if (origband && out) *out = origband;
	return ret;
}

BOOL WINAPI GetUserObjectInformationNew(HANDLE hObj, int nIndex, PVOID pvInfo, DWORD nLength, LPDWORD lpnLengthNeeded)
{
	lstrcpy(LPWSTR(pvInfo), L"Winlogon");
	return TRUE;
}

UINT_PTR WINAPI SetTimer_WUI(HWND hWnd, UINT_PTR nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc)
{
	if (nIDEvent == 0x2252CE37)
		ShowWindow(hWnd, SW_HIDE);
	return SetTimer(hWnd, nIDEvent, uElapse, lpTimerFunc);
}

void HookImmersive()
{
	HMODULE rghmodHookDlls[] = {
		LoadLibraryW(L"Windows.UI.Immersive.dll"),
		LoadLibraryW(L"TWinUI.PCShell.dll"),
	};

	HMODULE hUser32 = GetModuleHandle(L"user32.dll");
	CreateWindowInBandOrig = (CreateWindowInBandAPI)GetProcAddress(hUser32, "CreateWindowInBand");

	if (s_EnableImmersiveShellStack == 1)
		CreateWindowInBandExOrig = (CreateWindowInBandExAPI)GetProcAddress(hUser32, "CreateWindowInBand");

	GetWindowBandOrig = (GetWindowBandAPI)GetProcAddress(hUser32, "GetWindowBand");

	for (HMODULE &hmod : rghmodHookDlls)
	{
		ChangeImportedAddress(hmod, "user32.dll", CreateWindowInBandOrig, CreateWindowInBandNew);
		ChangeImportedAddress(hmod, "user32.dll", GetWindowBandOrig, GetWindowBandNew);
		ChangeImportedAddress(hmod, "user32.dll", GetUserObjectInformation, GetUserObjectInformationNew);
		ChangeImportedAddress(hmod, "user32.dll", SetTimer, SetTimer_WUI);
	}

	//if (!s_EnableImmersiveShellStack || g_osVersion.BuildNumber() < 10074) // Ittr: If user *either* has UWP disabled, or they are NOT on Windows 10, run legacy window band code
	//{
	//	//bugbug!!!
	//	ChangeImportedAddress(GetModuleHandle(L"twinui.dll"), "user32.dll", CreateWindowInBandOrig, CreateWindowInBandNew);
	//	ChangeImportedAddress(GetModuleHandle(L"authui.dll"), "user32.dll", CreateWindowInBandOrig, CreateWindowInBandNew);
	//	ChangeImportedAddress(GetModuleHandle(L"shell32.dll"), "user32.dll", CreateWindowInBandOrig, CreateWindowInBandNew);

	//	ChangeImportedAddress(GetModuleHandle(L"twinapi.dll"), "user32.dll", CreateWindowInBandOrig, CreateWindowInBandNew);
	//	ChangeImportedAddress(GetModuleHandle(L"Windows.UI.dll"), "user32.dll", CreateWindowInBandOrig, CreateWindowInBandNew);
	//}
}