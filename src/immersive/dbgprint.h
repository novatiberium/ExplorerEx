#pragma once
#include <windows.h>

void dbgvprintf(LPCWSTR format, void* _argp);
void dbgprintf(LPCWSTR format, ...);