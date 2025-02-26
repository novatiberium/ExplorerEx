#include "ImmersiveCommon.h"

void dbgvprintf(LPCWSTR format, void* _argp)
{
    WCHAR msg[1024];
    va_list argp = (va_list)_argp;
    int cnt = wvsprintfW(msg, format, argp);

    OutputDebugStringW(msg);
}

void dbgvprintfA(LPCSTR format, void* _argp)
{
    CHAR msg[1024];
    va_list argp = (va_list)_argp;
    int cnt = wvsprintfA(msg, format, argp);

    OutputDebugStringA(msg);
}

void dbgprintf(LPCWSTR format, ...)
{
    //format = wcscat(const_cast<wchar_t*>(format),L"\n");
    va_list argp;
    va_start(argp, format);
    dbgvprintf(format, argp);
    va_end(argp);
}

void dbgprintfA(LPCSTR format, ...)
{
    //format = wcscat(const_cast<wchar_t*>(format),L"\n");
    va_list argp;
    va_start(argp, format);
    dbgvprintfA(format, argp);
    va_end(argp);
}