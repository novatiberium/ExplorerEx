#pragma once

#ifndef _STARTBTN_H
#define _STARTBTN_H
#include <Windows.h>
#include <Uxtheme.h>
#include <ShObjIdl_core.h>

#include "tray.h"

// from explorer\desktop2
STDAPI DesktopV2_Create(
    IMenuPopup** ppmp, IMenuBand** ppmb, void** ppvStartPane);
STDAPI DesktopV2_Build(void* pvStartPane);

// from tray
EXTERN_C BOOL WINAPI Tray_StartPanelEnabled();



MIDL_INTERFACE("8B62940C-7ED5-4DE6-9BDC-4CA4346AAE3B")
IStartButton : IUnknown
{
    STDMETHOD(SetFocusToStartButton()) PURE;
    STDMETHOD(OnContextMenu(HWND hWnd, LPARAM lParam)) PURE;
    STDMETHOD(CreateStartButtonBalloon(UINT a2, UINT uID)) PURE;
    STDMETHOD(SetStartPaneActive(BOOL bActive)) PURE;
    STDMETHOD(OnStartMenuDismissed()) PURE;
    STDMETHOD(UnlockStartPane()) PURE;
    STDMETHOD(LockStartPane()) PURE;
    STDMETHOD(GetPopupPosition(DWORD* out)) PURE;
    STDMETHOD(GetWindow(HWND* hWndOut)) PURE;
};

class CStartButton : public IStartButton, public IServiceProvider
{
public:
    CStartButton();

    //~ Begin IUnknown Interface
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject) override;
    STDMETHODIMP_(ULONG) AddRef() override;
    STDMETHODIMP_(ULONG) Release() override;
    //~ End IUnknown Interface

    //~ Begin IStartButton Interface
    STDMETHODIMP SetFocusToStartButton() override;
    STDMETHODIMP OnContextMenu(HWND, LPARAM) override;
    STDMETHODIMP CreateStartButtonBalloon(UINT a2, UINT uID) override;
    STDMETHODIMP SetStartPaneActive(BOOL bActive) override;
    STDMETHODIMP OnStartMenuDismissed() override;
    STDMETHODIMP UnlockStartPane() override;
    STDMETHODIMP LockStartPane() override;
    STDMETHODIMP GetPopupPosition(DWORD* out) override;
    STDMETHODIMP GetWindow(HWND* out) override;
    //~ End IStartButton Interface

    //~ Begin IServiceProvider Interface
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void** ppvObject) override;
    //~ End IServiceProvider Interface

    // TODO: revise

    void BuildStartMenu();
    void CloseStartMenu();
    HWND CreateStartButton(HWND hwnd);
    void DestroyStartMenu();
    void DisplayStartMenu();
    void DrawStartButton(int iStateId, bool hdcSrc /*allegedly*/);
    void ExecRefresh();
    void ForceButtonUp();
    void GetRect(RECT* lpRect);
    void GetSizeAndFont(HTHEME hTheme);
    BOOL InitBackgroundBitmap();
    void InitTheme();
    BOOL IsButtonPushed();
    HRESULT IsMenuMessage(MSG* pmsg);
    BOOL IsPopupMenuVisible();
    void RecalcSize();
    void RepositionBalloon();
    void StartButtonReset();
    int TrackMenu(HMENU hMenu);
    BOOL TranslateMenuMessage(MSG* pmsg, LRESULT* plRet);
    void UpdateStartButton(bool a2 /*allegedly*/);
    void _DestroyStartButtonBalloon();
    void _DontShowTheStartButtonBalloonAnyMore();

    LPCWSTR _pszCurrentThemeName;
    INT _nSomeSize;
    int _nUnkBool2;
    int _nUnkBool3;
    HWND _hwndStartBtn;
    HWND _hwndStartBalloon;
    SIZE _size;
    HTHEME _hTheme;
    HBITMAP _hbmpStartBkg;
    HFONT _hStartFont;
    char padding[4];
    int _nUnkBool1;
    int _nStartBtnNotPressed;
    int _nIsOnContextMenu;
    BOOL _uLockCode;
    BOOL _fBackgroundBitmapInitialized;
    bool _nSettingsChangeType;
    int _nStartPaneActiveState;
    DWORD _dwTickCount;
    HIMAGELIST _hIml;
    IStartButtonSite *_pStartButtonSite;
    IMenuBand *_pOldStartMenuBand;
    IMenuPopup *_pOldStartMenu;
    IMenuBand *_pNewStartMenuBand;
    IMenuPopup *_pNewStartMenu;
    IDeskBand *_pUnk1;
    char padding5[4];
    LPWSTR _pszWindowName;

    // set in constructor passed from CTray
    UINT g_traystuckplace; // _uStuckPlace

    WCHAR WindowName;

private:

    LRESULT OnMouseClick(HWND hWndTo, LPARAM lParam);
    void _CalcExcludeRect(RECTL* lprcDst);
    BOOL _CalcStartButtonPos(POINT* a2, HRGN* a3);
    HFONT _CreateStartFont();
    void _ExploreCommonStartMenu(BOOL bExplore);

    LPCWSTR _GetCurrentThemeName();
    
    void _HandleDestroy();
    void _OnSettingChanged(UINT a2);
    bool _OnThemeChanged(bool bForceUpdate);
    BOOL _ShouldDelayClip(const RECT* a2, const RECT* lprcSrc2);
    LRESULT _StartButtonSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    static LRESULT s_StartButtonSubclassProc(
        HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
    static LRESULT s_StartMenuSubclassProc(
        HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
};


#endif 