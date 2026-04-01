#include "scwrapper.h"
#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <imm.h>
#include <cstdint>
#include <tlhelp32.h>
#include <dwmapi.h>

#include <external/xorstr.hpp>
#include <intrin.h>

#ifdef __cplusplus
extern "C" {
#endif

    // Хелпер для разрешения Forwarded функций
    void* ForceResolve(uint32_t modHash, const char* modName, uint32_t funcHash, const char* funcName) {
        void* ptr = GetSafeApi(modHash, modName, funcHash);
        if (ptr) return ptr;

        using tGetMod = HMODULE(WINAPI*)(LPCWSTR);
        using tGetProc = FARPROC(WINAPI*)(HMODULE, LPCSTR);

        static auto pGetMod = (tGetMod)GetSafeApi(H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("GetModuleHandleW"));
        static auto pGetProc = (tGetProc)GetSafeApi(H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("GetProcAddress"));

        if (pGetMod && pGetProc) {
            wchar_t wModName[64];
            for (int i = 0; i < 64; ++i) {
                wModName[i] = (wchar_t)modName[i];
                if (!modName[i]) break;
            }
            HMODULE hMod = pGetMod(wModName);
            if (hMod) {
                return (void*)pGetProc(hMod, funcName);
            }
        }
        return nullptr;
    }

    // === ИЗМЕНЕННЫЙ МАКРОС: ИСПОЛЬЗУЕТ SC_OBF_CALL ===
#define SYS_CALL_FORCE(TYPE, MOD_HASH, MOD_STR, FUNC_HASH, FUNC_STR, ...) \
        static auto pFunc = (TYPE)nullptr; \
        if (!pFunc) { \
            pFunc = (TYPE)ForceResolve(MOD_HASH, MOD_STR, FUNC_HASH, FUNC_STR); \
        } \
        if (pFunc) return SC_OBF_CALL(pFunc)(__VA_ARGS__); \
        return {}; 
    // ==================================================

    HRESULT pre_sys_D2D1CreateFactory_Raw(
        D2D1_FACTORY_TYPE factoryType,
        REFIID riid,
        const D2D1_FACTORY_OPTIONS* pFactoryOptions,
        void** ppIFactory
    ) {
        using tFunc = HRESULT(WINAPI*)(D2D1_FACTORY_TYPE, REFIID, const D2D1_FACTORY_OPTIONS*, void**);
        SYS_CALL(tFunc, H_D2D1, ecrypt("d2d1.dll"), CT_HASH("D2D1CreateFactory"),
            factoryType, riid, pFactoryOptions, ppIFactory
        );
    }

    HRESULT pre_sys_DWriteCreateFactory_Raw(
        DWRITE_FACTORY_TYPE factoryType,
        REFIID iid,
        IUnknown** factory
    ) {
        using tFunc = HRESULT(WINAPI*)(DWRITE_FACTORY_TYPE, REFIID, IUnknown**);
        SYS_CALL(tFunc, H_DWRITE, ecrypt("dwrite.dll"), CT_HASH("DWriteCreateFactory"),
            factoryType, iid, factory
        );
    }

    // --- KERNEL32 ---


    HANDLE pre_sys_GetCurrentThread() {
        // Сигнатура функции: HANDLE WINAPI GetCurrentThread(void);
        using tFunc = HANDLE(WINAPI*)();

        // Вызов через макрос обфускации
        SYS_CALL(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("GetCurrentThread"));
    }

    BOOL pre_sys_GetThreadContext(HANDLE hThread, LPCONTEXT lpContext) {
        // Сигнатура функции: BOOL WINAPI GetThreadContext(HANDLE, LPCONTEXT);
        using tFunc = BOOL(WINAPI*)(HANDLE, LPCONTEXT);

        // Вызов через макрос обфускации
        SYS_CALL(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("GetThreadContext"),
            hThread, lpContext
        );
    }

    BOOL pre_sys_QueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount) {
        using tFunc = BOOL(WINAPI*)(LARGE_INTEGER*);
        SYS_CALL(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("QueryPerformanceCounter"), lpPerformanceCount);
    }


    BOOL pre_sys_QueryPerformanceFrequency(LARGE_INTEGER* lpFrequency) {
        using tFunc = BOOL(WINAPI*)(LARGE_INTEGER*);
        SYS_CALL(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("QueryPerformanceFrequency"), lpFrequency);
    }

    HGLOBAL pre_sys_GlobalAlloc(UINT uFlags, SIZE_T dwBytes) {
        using tFunc = HGLOBAL(WINAPI*)(UINT, SIZE_T);
        SYS_CALL(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("GlobalAlloc"), uFlags, dwBytes);
    }

    HGLOBAL pre_sys_GlobalFree(HGLOBAL hMem) {
        using tFunc = HGLOBAL(WINAPI*)(HGLOBAL);
        SYS_CALL(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("GlobalFree"), hMem);
    }

    LPVOID pre_sys_GlobalLock(HGLOBAL hMem) {
        using tFunc = LPVOID(WINAPI*)(HGLOBAL);
        SYS_CALL(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("GlobalLock"), hMem);
    }

    BOOL pre_sys_GlobalUnlock(HGLOBAL hMem) {
        using tFunc = BOOL(WINAPI*)(HGLOBAL);
        SYS_CALL(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("GlobalUnlock"), hMem);
    }

    int pre_sys_WideCharToMultiByte(UINT CodePage, DWORD dwFlags, LPCWCH lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte, LPCCH lpDefaultChar, LPBOOL lpUsedDefaultChar) {
        using tFunc = int(WINAPI*)(UINT, DWORD, LPCWCH, int, LPSTR, int, LPCCH, LPBOOL);
        SYS_CALL(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("WideCharToMultiByte"), CodePage, dwFlags, lpWideCharStr, cchWideChar, lpMultiByteStr, cbMultiByte, lpDefaultChar, lpUsedDefaultChar);
    }

    BOOL pre_sys_SetConsoleTitleA(LPCSTR lpConsoleTitle) {
        using tFunc = BOOL(WINAPI*)(LPCSTR);
        SYS_CALL(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("SetConsoleTitleA"), lpConsoleTitle);
    }

    HANDLE pre_sys_GetStdHandle(DWORD nStdHandle) {
        using tFunc = HANDLE(WINAPI*)(DWORD);
        SYS_CALL(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("GetStdHandle"), nStdHandle);
    }

    BOOL pre_sys_SetConsoleMode(HANDLE hConsoleHandle, DWORD dwMode) {
        using tFunc = BOOL(WINAPI*)(HANDLE, DWORD);
        SYS_CALL(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("SetConsoleMode"), hConsoleHandle, dwMode);
    }

    BOOL pre_sys_GetConsoleMode(HANDLE hConsoleHandle, LPDWORD lpMode) {
        using tFunc = BOOL(WINAPI*)(HANDLE, LPDWORD);
        SYS_CALL(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("GetConsoleMode"), hConsoleHandle, lpMode);
    }

    BOOL pre_sys_GetConsoleCursorInfo(HANDLE hConsoleOutput, PCONSOLE_CURSOR_INFO lpConsoleCursorInfo) {
        using tFunc = BOOL(WINAPI*)(HANDLE, PCONSOLE_CURSOR_INFO);
        SYS_CALL(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("GetConsoleCursorInfo"), hConsoleOutput, lpConsoleCursorInfo);
    }

    BOOL pre_sys_SetConsoleCursorInfo(HANDLE hConsoleOutput, const CONSOLE_CURSOR_INFO* lpConsoleCursorInfo) {
        using tFunc = BOOL(WINAPI*)(HANDLE, const CONSOLE_CURSOR_INFO*);
        SYS_CALL(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("SetConsoleCursorInfo"), hConsoleOutput, lpConsoleCursorInfo);
    }

    BOOL pre_sys_SetCurrentConsoleFontEx(HANDLE hConsoleOutput, BOOL bMaximumWindow, PCONSOLE_FONT_INFOEX lpConsoleCurrentFontEx) {
        using tFunc = BOOL(WINAPI*)(HANDLE, BOOL, PCONSOLE_FONT_INFOEX);
        SYS_CALL(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("SetCurrentConsoleFontEx"), hConsoleOutput, bMaximumWindow, lpConsoleCurrentFontEx);
    }

    BOOL pre_sys_GetCurrentConsoleFontEx(HANDLE hConsoleOutput, BOOL bMaximumWindow, PCONSOLE_FONT_INFOEX lpConsoleCurrentFontEx) {
        using tFunc = BOOL(WINAPI*)(HANDLE, BOOL, PCONSOLE_FONT_INFOEX);
        SYS_CALL(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("GetCurrentConsoleFontEx"), hConsoleOutput, bMaximumWindow, lpConsoleCurrentFontEx);
    }

    BOOL pre_sys_FlushConsoleInputBuffer(HANDLE hConsoleInput) {
        using tFunc = BOOL(WINAPI*)(HANDLE);
        SYS_CALL(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("FlushConsoleInputBuffer"), hConsoleInput);
    }

    int pre_sys_GetLocaleInfoA(LCID Locale, LCTYPE LCType, LPSTR lpLCData, int cchData) {
        using tFunc = int(WINAPI*)(LCID, LCTYPE, LPSTR, int);
        SYS_CALL(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("GetLocaleInfoA"), Locale, LCType, lpLCData, cchData);
    }

    VOID pre_sys_GetSystemTimeAsFileTime(LPFILETIME lpSystemTimeAsFileTime) {
        using tFunc = VOID(WINAPI*)(LPFILETIME);
        SYS_CALL_VOID(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("GetSystemTimeAsFileTime"), lpSystemTimeAsFileTime);
    }

    DWORD pre_sys_GetCurrentProcessId(void) {
        using tFunc = DWORD(WINAPI*)(void);
        SYS_CALL(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("GetCurrentProcessId"));
    }

    DWORD pre_sys_GetCurrentThreadId(void) {
        using tFunc = DWORD(WINAPI*)(void);
        SYS_CALL(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("GetCurrentThreadId"));
    }

    HMODULE pre_sys_GetModuleHandleW(LPCWSTR lpModuleName) {
        using tFunc = HMODULE(WINAPI*)(LPCWSTR);
        SYS_CALL(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("GetModuleHandleW"), lpModuleName);
    }

    LPTOP_LEVEL_EXCEPTION_FILTER pre_sys_SetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter) {
        using tFunc = LPTOP_LEVEL_EXCEPTION_FILTER(WINAPI*)(LPTOP_LEVEL_EXCEPTION_FILTER);
        SYS_CALL(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("SetUnhandledExceptionFilter"), lpTopLevelExceptionFilter);
    }

    VOID pre_sys_WakeAllConditionVariable(PCONDITION_VARIABLE ConditionVariable) {
        using tFunc = VOID(WINAPI*)(PCONDITION_VARIABLE);
        SYS_CALL_VOID(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("WakeAllConditionVariable"), ConditionVariable);
    }

    VOID pre_sys_Sleep(DWORD dwMilliseconds) {
        using tFunc = VOID(WINAPI*)(DWORD);
        SYS_CALL_VOID(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("Sleep"), dwMilliseconds);
    }

    BOOL pre_sys_SleepConditionVariableSRW(PCONDITION_VARIABLE ConditionVariable, PSRWLOCK SRWLock, DWORD dwMilliseconds, ULONG Flags) {
        using tFunc = BOOL(WINAPI*)(PCONDITION_VARIABLE, PSRWLOCK, DWORD, ULONG);
        SYS_CALL(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("SleepConditionVariableSRW"), ConditionVariable, SRWLock, dwMilliseconds, Flags);
    }

    VOID pre_sys_AcquireSRWLockExclusive(PSRWLOCK SRWLock) {
        using tFunc = VOID(WINAPI*)(PSRWLOCK);
        SYS_CALL_VOID(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("AcquireSRWLockExclusive"), SRWLock);
    }

    VOID pre_sys_ReleaseSRWLockExclusive(PSRWLOCK SRWLock) {
        using tFunc = VOID(WINAPI*)(PSRWLOCK);
        SYS_CALL_VOID(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("ReleaseSRWLockExclusive"), SRWLock);
    }

    BOOL pre_sys_FreeLibrary(HMODULE hLibModule) {
        using tFunc = BOOL(WINAPI*)(HMODULE);
        SYS_CALL(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("FreeLibrary"), hLibModule);
    }

    FARPROC pre_sys_GetProcAddress(HMODULE hModule, LPCSTR lpProcName) {
        using tFunc = FARPROC(WINAPI*)(HMODULE, LPCSTR);
        SYS_CALL(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("GetProcAddress"), hModule, lpProcName);
    }

    HMODULE pre_sys_LoadLibraryA(LPCSTR lpLibFileName) {
        using tFunc = HMODULE(WINAPI*)(LPCSTR);
        SYS_CALL(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("LoadLibraryA"), lpLibFileName);
    }

    VOID pre_sys_InitializeSListHead(PSLIST_HEADER ListHead) {
        using tFunc = VOID(WINAPI*)(PSLIST_HEADER);
        SYS_CALL_VOID(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("InitializeSListHead"), ListHead);
    }

    DWORD pre_sys_GetFullPathNameA(LPCSTR lpFileName, DWORD nBufferLength, LPSTR lpBuffer, LPSTR* lpFilePart) {
        using tFunc = DWORD(WINAPI*)(LPCSTR, DWORD, LPSTR, LPSTR*);
        SYS_CALL(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("GetFullPathNameA"), lpFileName, nBufferLength, lpBuffer, lpFilePart);
    }

    DWORD pre_sys_GetModuleFileNameA(HMODULE hModule, LPSTR lpFilename, DWORD nSize) {
        using tFunc = DWORD(WINAPI*)(HMODULE, LPSTR, DWORD);
        SYS_CALL(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("GetModuleFileNameA"), hModule, lpFilename, nSize);
    }

    int pre_sys_MultiByteToWideChar(UINT CodePage, DWORD dwFlags, LPCCH lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar) {
        using tFunc = int(WINAPI*)(UINT, DWORD, LPCCH, int, LPWSTR, int);
        SYS_CALL(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("MultiByteToWideChar"), CodePage, dwFlags, lpMultiByteStr, cbMultiByte, lpWideCharStr, cchWideChar);
    }

    HWND pre_sys_GetConsoleWindow(void) {
        using tFunc = HWND(WINAPI*)(void);
        SYS_CALL(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("GetConsoleWindow"));
    }

    UINT pre_sys_timeBeginPeriod(UINT uPeriod) {
        // MMRESULT is actually just a typedef for UINT in mmsystem.h
        using tFunc = UINT(WINAPI*)(UINT);

        // Resolve winmm.dll dynamically and call the function
        SYS_CALL(tFunc, CT_HASHW(L"winmm.dll"), ecrypt("winmm.dll"), CT_HASH("timeBeginPeriod"), uPeriod);
    }
    // --- USER32 ---

    LONG pre_sys_GetWindowLongA(HWND hWnd, int nIndex) {
        using tFunc = LONG(WINAPI*)(HWND, int);
        SYS_CALL_FORCE(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("GetWindowLongA"), ecrypt("GetWindowLongA"), hWnd, nIndex);
    }

    BOOL pre_sys_SetLayeredWindowAttributes(HWND hwnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags) {
        using tFunc = BOOL(WINAPI*)(HWND, COLORREF, BYTE, DWORD);
        SYS_CALL_FORCE(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("SetLayeredWindowAttributes"), ecrypt("SetLayeredWindowAttributes"), hwnd, crKey, bAlpha, dwFlags);
    }

    //fixed
    HCURSOR pre_sys_LoadCursorA(HINSTANCE hInstance, LPCSTR lpCursorName) {
        using tFunc = HCURSOR(WINAPI*)(HINSTANCE, LPCSTR);
        SYS_CALL_FORCE(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("LoadCursorA"), ecrypt("LoadCursorA"), hInstance, lpCursorName);
    }
/*
    HCURSOR pre_sys_LoadCursorA(HINSTANCE hInstance, LPCSTR lpCursorName) {
        using tFunc = HCURSOR(WINAPI*)(HINSTANCE, LPCSTR);
        SYS_CALL(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("LoadCursorA"), hInstance, lpCursorName);
    }
*/

    BOOL pre_sys_ClientToScreen(HWND hWnd, LPPOINT lpPoint) {
        using tFunc = BOOL(WINAPI*)(HWND, LPPOINT);
        SYS_CALL_FORCE(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("ClientToScreen"), ecrypt("ClientToScreen"), hWnd, lpPoint);
    }
    
    //fixed
    int pre_sys_GetSystemMetrics(int nIndex) {
        using tFunc = int(WINAPI*)(int);
        SYS_CALL_FORCE(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("GetSystemMetrics"), ecrypt("GetSystemMetrics"), nIndex);
    }
/*
    int pre_sys_GetSystemMetrics(int nIndex) {
        using tFunc = int(WINAPI*)(int);
        SYS_CALL(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("GetSystemMetrics"), nIndex);
    }
*/

    BOOL pre_sys_IsWindow(HWND hWnd) {
        using tFunc = BOOL(WINAPI*)(HWND);
        SYS_CALL(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("IsWindow"), hWnd);
    }

    BOOL pre_sys_ScreenToClient(HWND hWnd, LPPOINT lpPoint) {
        using tFunc = BOOL(WINAPI*)(HWND, LPPOINT);
        SYS_CALL_FORCE(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("ScreenToClient"), ecrypt("ScreenToClient"), hWnd, lpPoint);
    }

    BOOL pre_sys_GetClientRect(HWND hWnd, LPRECT lpRect) {
        using tFunc = BOOL(WINAPI*)(HWND, LPRECT);
        SYS_CALL_FORCE(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("GetClientRect"), ecrypt("GetClientRect"), hWnd, lpRect);
    }

    HHOOK pre_sys_SetWindowsHookExA(int idHook, HOOKPROC lpfn, HINSTANCE hMod, DWORD dwThreadId) {
        using tFunc = HHOOK(WINAPI*)(int, HOOKPROC, HINSTANCE, DWORD);
        SYS_CALL(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("SetWindowsHookExA"), idHook, lpfn, hMod, dwThreadId);
    }

    //fixed
    BOOL pre_sys_ShowWindow(HWND hWnd, int nCmdShow) {
        using tFunc = BOOL(WINAPI*)(HWND, int);
        SYS_CALL_FORCE(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("ShowWindow"), ecrypt("ShowWindow"), hWnd, nCmdShow);
    }
/*
    BOOL pre_sys_ShowWindow(HWND hWnd, int nCmdShow) {
        using tFunc = BOOL(WINAPI*)(HWND, int);
        SYS_CALL(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("ShowWindow"), hWnd, nCmdShow);
    }
*/

    LONG pre_sys_SetWindowLongA(HWND hWnd, int nIndex, LONG dwNewLong) {
        using tFunc = LONG(WINAPI*)(HWND, int, LONG);
        SYS_CALL_FORCE(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("SetWindowLongA"), ecrypt("SetWindowLongA"), hWnd, nIndex, dwNewLong);
    }

    LONG pre_sys_SetWindowLongW(HWND hWnd, int nIndex, LONG dwNewLong) {
        using tFunc = LONG(WINAPI*)(HWND, int, LONG);
        SYS_CALL_FORCE(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("SetWindowLongW"), ecrypt("SetWindowLongW"), hWnd, nIndex, dwNewLong);
    }

    HWND pre_sys_FindWindowA(LPCSTR lpClassName, LPCSTR lpWindowName) {
        using tFunc = HWND(WINAPI*)(LPCSTR, LPCSTR);
        SYS_CALL(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("FindWindowA"), lpClassName, lpWindowName);
    }

    HWND pre_sys_FindWindowExA(HWND hWndParent, HWND hWndChildAfter, LPCSTR lpszClass, LPCSTR lpszWindow) {
        using tFunc = HWND(WINAPI*)(HWND, HWND, LPCSTR, LPCSTR);
        SYS_CALL(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("FindWindowExA"), hWndParent, hWndChildAfter, lpszClass, lpszWindow);
    }

    BOOL pre_sys_IsWindowVisible(HWND hWnd) {
        using tFunc = BOOL(WINAPI*)(HWND);
        SYS_CALL(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("IsWindowVisible"), hWnd);
    }

    HWND pre_sys_GetForegroundWindow(void) {
        using tFunc = HWND(WINAPI*)(void);
        SYS_CALL(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("GetForegroundWindow"));
    }

    SHORT pre_sys_GetAsyncKeyState(int vKey) {
        using tFunc = SHORT(WINAPI*)(int);
        SYS_CALL(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("GetAsyncKeyState"), vKey);
    }

    SHORT pre_sys_GetKeyState(int nVirtKey) {
        using tFunc = SHORT(WINAPI*)(int);
        SYS_CALL(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("GetKeyState"), nVirtKey);
    }

    BOOL pre_sys_GetKeyboardState(PBYTE lpKeyState) {
        using tFunc = BOOL(WINAPI*)(PBYTE);
        SYS_CALL(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("GetKeyboardState"), lpKeyState);
    }

    int pre_sys_ToUnicode(UINT wVirtKey, UINT wScanCode, const BYTE* lpKeyState, LPWSTR pwszBuff, int cchBuff, UINT wFlags) {
        using tFunc = int(WINAPI*)(UINT, UINT, const BYTE*, LPWSTR, int, UINT);
        SYS_CALL(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("ToUnicode"), wVirtKey, wScanCode, lpKeyState, pwszBuff, cchBuff, wFlags);
    }

    HKL pre_sys_GetKeyboardLayout(DWORD idThread) {
        using tFunc = HKL(WINAPI*)(DWORD);
        SYS_CALL(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("GetKeyboardLayout"), idThread);
    }

    BOOL pre_sys_SetCursorPos(int X, int Y) {
        using tFunc = BOOL(WINAPI*)(int, int);
        SYS_CALL(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("SetCursorPos"), X, Y);
    }

    BOOL pre_sys_GetCursorPos(LPPOINT lpPoint) {
        using tFunc = BOOL(WINAPI*)(LPPOINT);
        SYS_CALL_FORCE(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("GetCursorPos"), ecrypt("GetCursorPos"), lpPoint);
    }

    HCURSOR pre_sys_SetCursor(HCURSOR hCursor) {
        using tFunc = HCURSOR(WINAPI*)(HCURSOR);
        SYS_CALL(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("SetCursor"), hCursor);
    }

    int pre_sys_ShowCursor(BOOL bShow) {
        using tFunc = int(WINAPI*)(BOOL);
        SYS_CALL(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("ShowCursor"), bShow);
    }

    BOOL pre_sys_PeekMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg) {
        using tFunc = BOOL(WINAPI*)(LPMSG, HWND, UINT, UINT, UINT);
        SYS_CALL_FORCE(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("PeekMessageA"), ecrypt("PeekMessageA"), lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
    }

    LRESULT pre_sys_DispatchMessageA(const MSG* lpMsg) {
        using tFunc = LRESULT(WINAPI*)(const MSG*);
        SYS_CALL_FORCE(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("DispatchMessageA"), ecrypt("DispatchMessageA"), lpMsg);
    }

    BOOL pre_sys_TranslateMessage(const MSG* lpMsg) {
        using tFunc = BOOL(WINAPI*)(const MSG*);
        SYS_CALL_FORCE(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("TranslateMessage"), ecrypt("TranslateMessage"), lpMsg);
    }

    LRESULT pre_sys_CallNextHookEx(HHOOK hhk, int nCode, WPARAM wParam, LPARAM lParam) {
        using tFunc = LRESULT(WINAPI*)(HHOOK, int, WPARAM, LPARAM);
        SYS_CALL(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("CallNextHookEx"), hhk, nCode, wParam, lParam);
    }

    BOOL pre_sys_UnhookWindowsHookEx(HHOOK hhk) {
        using tFunc = BOOL(WINAPI*)(HHOOK);
        SYS_CALL(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("UnhookWindowsHookEx"), hhk);
    }

    BOOL pre_sys_OpenClipboard(HWND hWndNewOwner) {
        using tFunc = BOOL(WINAPI*)(HWND);
        SYS_CALL(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("OpenClipboard"), hWndNewOwner);
    }

    BOOL pre_sys_CloseClipboard(void) {
        using tFunc = BOOL(WINAPI*)(void);
        SYS_CALL(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("CloseClipboard"));
    }

    BOOL pre_sys_EmptyClipboard(void) {
        using tFunc = BOOL(WINAPI*)(void);
        SYS_CALL(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("EmptyClipboard"));
    }

    HANDLE pre_sys_GetClipboardData(UINT uFormat) {
        using tFunc = HANDLE(WINAPI*)(UINT);
        SYS_CALL(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("GetClipboardData"), uFormat);
    }

    HANDLE pre_sys_SetClipboardData(UINT uFormat, HANDLE hMem) {
        using tFunc = HANDLE(WINAPI*)(UINT, HANDLE);
        SYS_CALL(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("SetClipboardData"), uFormat, hMem);
    }

    HANDLE pre_sys_SWDA(HWND hWnd, DWORD dwAffinity) {
        typedef BOOL(WINAPI* tSetWindowDisplayAffinity)(HWND, DWORD);
        static tSetWindowDisplayAffinity pFunc = nullptr;
        if (!pFunc) {
            pFunc = (tSetWindowDisplayAffinity)GetSafeApi(H_USER32, ecrypt("user32.dll"), CT_HASH("SetWindowDisplayAffinity"));
        }
        if (pFunc) {
            BOOL res = SC_OBF_CALL(pFunc)(hWnd, dwAffinity);
            return (HANDLE)(uintptr_t)res;
        }
        return NULL;
    }

    // --- SHELL32 ---

    HINSTANCE pre_sys_ShellExecuteW(HWND hwnd, LPCWSTR lpOperation, LPCWSTR lpFile, LPCWSTR lpParameters, LPCWSTR lpDirectory, INT nShowCmd) {
        using tFunc = HINSTANCE(WINAPI*)(HWND, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR, INT);
        SYS_CALL(tFunc, H_SHELL32, ecrypt("shell32.dll"), CT_HASH("ShellExecuteW"), hwnd, lpOperation, lpFile, lpParameters, lpDirectory, nShowCmd);
    }

    // --- IMM32 ---

    BOOL pre_sys_ImmSetCompositionWindow(HIMC hIMC, LPCOMPOSITIONFORM lpCompForm) {
        using tFunc = BOOL(WINAPI*)(HIMC, LPCOMPOSITIONFORM);
        SYS_CALL(tFunc, H_IMM32, ecrypt("imm32.dll"), CT_HASH("ImmSetCompositionWindow"), hIMC, lpCompForm);
    }

    BOOL pre_sys_ImmSetCandidateWindow(HIMC hIMC, LPCANDIDATEFORM lpCandidate) {
        using tFunc = BOOL(WINAPI*)(HIMC, LPCANDIDATEFORM);
        SYS_CALL(tFunc, H_IMM32, ecrypt("imm32.dll"), CT_HASH("ImmSetCandidateWindow"), hIMC, lpCandidate);
    }

    HIMC pre_sys_ImmGetContext(HWND hWnd) {
        using tFunc = HIMC(WINAPI*)(HWND);
        SYS_CALL(tFunc, H_IMM32, ecrypt("imm32.dll"), CT_HASH("ImmGetContext"), hWnd);
    }

    BOOL pre_sys_ImmReleaseContext(HWND hWnd, HIMC hIMC) {
        using tFunc = BOOL(WINAPI*)(HWND, HIMC);
        SYS_CALL(tFunc, H_IMM32, ecrypt("imm32.dll"), CT_HASH("ImmReleaseContext"), hWnd, hIMC);
    }

    // --- D3D11 / D3DCOMPILER ---

    HRESULT pre_sys_D3D11CreateDeviceAndSwapChain(IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, const D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, IDXGISwapChain** ppSwapChain, ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext) {
        using tFunc = HRESULT(WINAPI*)(IDXGIAdapter*, D3D_DRIVER_TYPE, HMODULE, UINT, const D3D_FEATURE_LEVEL*, UINT, UINT, const DXGI_SWAP_CHAIN_DESC*, IDXGISwapChain**, ID3D11Device**, D3D_FEATURE_LEVEL*, ID3D11DeviceContext**);
        SYS_CALL(tFunc, H_D3D11, ecrypt("d3d11.dll"), CT_HASH("D3D11CreateDeviceAndSwapChain"), pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, pSwapChainDesc, ppSwapChain, ppDevice, pFeatureLevel, ppImmediateContext);
    }

    HRESULT pre_sys_D3DCompile(LPCVOID pSrcData, SIZE_T SrcDataSize, LPCSTR pSourceName, const D3D_SHADER_MACRO* pDefines, ID3DInclude* pInclude, LPCSTR pEntrypoint, LPCSTR pTarget, UINT Flags1, UINT Flags2, ID3DBlob** ppCode, ID3DBlob** ppErrorMsgs) {
        using tFunc = HRESULT(WINAPI*)(LPCVOID, SIZE_T, LPCSTR, const D3D_SHADER_MACRO*, ID3DInclude*, LPCSTR, LPCSTR, UINT, UINT, ID3DBlob**, ID3DBlob**);
        SYS_CALL(tFunc, H_D3DCOMP, ecrypt("d3dcompiler_47.dll"), CT_HASH("D3DCompile"), pSrcData, SrcDataSize, pSourceName, pDefines, pInclude, pEntrypoint, pTarget, Flags1, Flags2, ppCode, ppErrorMsgs);
    }

    // ================= ADVAPI32 =================

    BOOL pre_sys_OpenProcessToken(HANDLE ProcessHandle, DWORD DesiredAccess, PHANDLE TokenHandle) {
        using tFunc = BOOL(WINAPI*)(HANDLE, DWORD, PHANDLE);
        SYS_CALL(tFunc, H_ADVAPI32, ecrypt("advapi32.dll"), CT_HASH("OpenProcessToken"), ProcessHandle, DesiredAccess, TokenHandle);
    }

    BOOL pre_sys_GetTokenInformation(HANDLE TokenHandle, TOKEN_INFORMATION_CLASS TokenInformationClass, LPVOID TokenInformation, DWORD TokenInformationLength, PDWORD ReturnLength) {
        using tFunc = BOOL(WINAPI*)(HANDLE, TOKEN_INFORMATION_CLASS, LPVOID, DWORD, PDWORD);
        SYS_CALL(tFunc, H_ADVAPI32, ecrypt("advapi32.dll"), CT_HASH("GetTokenInformation"), TokenHandle, TokenInformationClass, TokenInformation, TokenInformationLength, ReturnLength);
    }

    BOOL pre_sys_LookupPrivilegeValueA(LPCSTR lpSystemName, LPCSTR lpName, PLUID lpLuid) {
        using tFunc = BOOL(WINAPI*)(LPCSTR, LPCSTR, PLUID);
        SYS_CALL(tFunc, H_ADVAPI32, ecrypt("advapi32.dll"), CT_HASH("LookupPrivilegeValueA"), lpSystemName, lpName, lpLuid);
    }

    BOOL pre_sys_PrivilegeCheck(HANDLE ClientToken, PPRIVILEGE_SET RequiredPrivileges, LPBOOL pfResult) {
        using tFunc = BOOL(WINAPI*)(HANDLE, PPRIVILEGE_SET, LPBOOL);
        SYS_CALL(tFunc, H_ADVAPI32, ecrypt("advapi32.dll"), CT_HASH("PrivilegeCheck"), ClientToken, RequiredPrivileges, pfResult);
    }

    BOOL pre_sys_DuplicateTokenEx(HANDLE hExistingToken, DWORD dwDesiredAccess, LPSECURITY_ATTRIBUTES lpTokenAttributes, SECURITY_IMPERSONATION_LEVEL ImpersonationLevel, TOKEN_TYPE TokenType, PHANDLE phNewToken) {
        using tFunc = BOOL(WINAPI*)(HANDLE, DWORD, LPSECURITY_ATTRIBUTES, SECURITY_IMPERSONATION_LEVEL, TOKEN_TYPE, PHANDLE);
        SYS_CALL(tFunc, H_ADVAPI32, ecrypt("advapi32.dll"), CT_HASH("DuplicateTokenEx"), hExistingToken, dwDesiredAccess, lpTokenAttributes, ImpersonationLevel, TokenType, phNewToken);
    }

    BOOL pre_sys_SetThreadToken(PHANDLE Thread, HANDLE Token) {
        using tFunc = BOOL(WINAPI*)(PHANDLE, HANDLE);
        SYS_CALL(tFunc, H_ADVAPI32, ecrypt("advapi32.dll"), CT_HASH("SetThreadToken"), Thread, Token);
    }

    BOOL pre_sys_SetTokenInformation(HANDLE TokenHandle, TOKEN_INFORMATION_CLASS TokenInformationClass, LPVOID TokenInformation, DWORD TokenInformationLength) {
        using tFunc = BOOL(WINAPI*)(HANDLE, TOKEN_INFORMATION_CLASS, LPVOID, DWORD);
        SYS_CALL(tFunc, H_ADVAPI32, ecrypt("advapi32.dll"), CT_HASH("SetTokenInformation"), TokenHandle, TokenInformationClass, TokenInformation, TokenInformationLength);
    }

    BOOL pre_sys_RevertToSelf(void) {
        using tFunc = BOOL(WINAPI*)(void);
        SYS_CALL(tFunc, H_ADVAPI32, ecrypt("advapi32.dll"), CT_HASH("RevertToSelf"));
    }

    BOOL pre_sys_CreateProcessAsUserA(HANDLE hToken, LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCSTR lpCurrentDirectory, LPSTARTUPINFOA lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation) {
        using tFunc = BOOL(WINAPI*)(HANDLE, LPCSTR, LPSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCSTR, LPSTARTUPINFOA, LPPROCESS_INFORMATION);
        SYS_CALL(tFunc, H_ADVAPI32, ecrypt("advapi32.dll"), CT_HASH("CreateProcessAsUserA"), hToken, lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
    }

    // ================= KERNEL32 EXTENSIONS =================

    BOOL pre_sys_CloseHandle(HANDLE hObject) {
        using tFunc = BOOL(WINAPI*)(HANDLE);
        SYS_CALL(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("CloseHandle"), hObject);
    }

    DWORD pre_sys_GetLastError(void) {
        using tFunc = DWORD(WINAPI*)(void);
        SYS_CALL(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("GetLastError"));
    }

    HANDLE pre_sys_CreateToolhelp32Snapshot(DWORD dwFlags, DWORD th32ProcessID) {
        using tFunc = HANDLE(WINAPI*)(DWORD, DWORD);
        SYS_CALL(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("CreateToolhelp32Snapshot"), dwFlags, th32ProcessID);
    }

    BOOL pre_sys_Process32First(HANDLE hSnapshot, LPPROCESSENTRY32 lppe) {
        using tFunc = BOOL(WINAPI*)(HANDLE, LPPROCESSENTRY32);
        SYS_CALL(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("Process32First"), hSnapshot, lppe);
    }

    BOOL pre_sys_Process32Next(HANDLE hSnapshot, LPPROCESSENTRY32 lppe) {
        using tFunc = BOOL(WINAPI*)(HANDLE, LPPROCESSENTRY32);
        SYS_CALL(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("Process32Next"), hSnapshot, lppe);
    }

    HANDLE pre_sys_OpenProcess(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwProcessId) {
        using tFunc = HANDLE(WINAPI*)(DWORD, BOOL, DWORD);
        SYS_CALL(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("OpenProcess"), dwDesiredAccess, bInheritHandle, dwProcessId);
    }

    VOID pre_sys_GetStartupInfoW(LPSTARTUPINFOW lpStartupInfo) {
        using tFunc = VOID(WINAPI*)(LPSTARTUPINFOW);
        SYS_CALL_VOID(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("GetStartupInfoW"), lpStartupInfo);
    }

    LPSTR pre_sys_GetCommandLineA(void) {
        using tFunc = LPSTR(WINAPI*)(void);
        SYS_CALL(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("GetCommandLineA"));
    }

    VOID pre_sys_ExitProcess(UINT uExitCode) {
        using tFunc = VOID(WINAPI*)(UINT);
        SYS_CALL_VOID(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("ExitProcess"), uExitCode);
    }

    HANDLE pre_sys_GetCurrentProcess(void) {
        using tFunc = HANDLE(WINAPI*)(void);
        SYS_CALL(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("GetCurrentProcess"));
    }

    BOOL pre_sys_SetPriorityClass(HANDLE hProcess, DWORD dwPriorityClass) {
        using tFunc = BOOL(WINAPI*)(HANDLE, DWORD);
        SYS_CALL(tFunc, H_KERNEL32, ecrypt("kernel32.dll"), CT_HASH("SetPriorityClass"), hProcess, dwPriorityClass);
    }

    // ================= USER32 EXTENSIONS =================

    ATOM pre_sys_RegisterClassExW(CONST WNDCLASSEXW* Arg1) {
        using tFunc = ATOM(WINAPI*)(CONST WNDCLASSEXW*);
        // Changed to SYS_CALL_FORCE to bypass export forwarding issues in user32.dll
        SYS_CALL_FORCE(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("RegisterClassExW"), ecrypt("RegisterClassExW"), Arg1);
    }
    BOOL pre_sys_UnregisterClassW(LPCWSTR lpClassName, HINSTANCE hInstance) {
        using tFunc = BOOL(WINAPI*)(LPCWSTR, HINSTANCE);
        SYS_CALL(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("UnregisterClassW"), lpClassName, hInstance);
    }

/*
    HWND pre_sys_CreateWindowExW(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) {
        using tFunc = HWND(WINAPI*)(DWORD, LPCWSTR, LPCWSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID);
        SYS_CALL(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("CreateWindowExW"), dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
    }
*/
    //fixed
    HWND pre_sys_CreateWindowExW(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) {
        using tFunc = HWND(WINAPI*)(DWORD, LPCWSTR, LPCWSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID);
        SYS_CALL_FORCE(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("CreateWindowExW"), ecrypt("CreateWindowExW"), dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
    }

    BOOL pre_sys_GetWindowRect(HWND hWnd, LPRECT lpRect) {
        using tFunc = BOOL(WINAPI*)(HWND, LPRECT);
        SYS_CALL(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("GetWindowRect"), hWnd, lpRect);
    }

    //fixed
    BOOL pre_sys_UpdateWindow(HWND hWnd) {
        using tFunc = BOOL(WINAPI*)(HWND);
        SYS_CALL_FORCE(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("UpdateWindow"), ecrypt("UpdateWindow"), hWnd);
    }
/*
    BOOL pre_sys_UpdateWindow(HWND hWnd) {
        using tFunc = BOOL(WINAPI*)(HWND);
        SYS_CALL(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("UpdateWindow"), hWnd);
    }
*/

    LONG_PTR pre_sys_GetWindowLongPtrA(HWND hWnd, int nIndex) {
        using tFunc = LONG_PTR(WINAPI*)(HWND, int);
#ifdef _WIN64
        SYS_CALL_FORCE(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("GetWindowLongPtrA"), ecrypt("GetWindowLongPtrA"), hWnd, nIndex);
#else
        using tFunc32 = LONG(WINAPI*)(HWND, int);
        SYS_CALL_FORCE((tFunc32), H_USER32, ecrypt("user32.dll"), CT_HASH("GetWindowLongA"), ecrypt("GetWindowLongA"), hWnd, nIndex);
#endif
    }

    LONG_PTR pre_sys_SetWindowLongPtrA(HWND hWnd, int nIndex, LONG_PTR dwNewLong) {
        using tFunc = LONG_PTR(WINAPI*)(HWND, int, LONG_PTR);
#ifdef _WIN64
        SYS_CALL_FORCE(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("SetWindowLongPtrA"), ecrypt("SetWindowLongPtrA"), hWnd, nIndex, dwNewLong);
#else
        using tFunc32 = LONG(WINAPI*)(HWND, int, LONG);
        SYS_CALL_FORCE((tFunc32), H_USER32, ecrypt("user32.dll"), CT_HASH("SetWindowLongA"), ecrypt("SetWindowLongA"), hWnd, nIndex, (LONG)dwNewLong);
#endif
    }

    BOOL pre_sys_DestroyWindow(HWND hWnd) {
        using tFunc = BOOL(WINAPI*)(HWND);
        SYS_CALL(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("DestroyWindow"), hWnd);
    }

    HWND pre_sys_SetActiveWindow(HWND hWnd) {
        using tFunc = HWND(WINAPI*)(HWND);
        SYS_CALL(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("SetActiveWindow"), hWnd);
    }

    BOOL pre_sys_SetForegroundWindow(HWND hWnd) {
        using tFunc = BOOL(WINAPI*)(HWND);
        SYS_CALL(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("SetForegroundWindow"), hWnd);
    }

    // === КРИТИЧЕСКОЕ ИСПРАВЛЕНИЕ ДЛЯ DefWindowProcA ===
    // Используем ForceResolve, чтобы обойти проверку на форвардеры в маппере
    LRESULT pre_sys_DefWindowProcA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
        using tFunc = LRESULT(WINAPI*)(HWND, UINT, WPARAM, LPARAM);
        SYS_CALL_FORCE(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("DefWindowProcA"), ecrypt("DefWindowProcA"), hWnd, Msg, wParam, lParam);
    }

    // 2. PostQuitMessage - Usually stays in user32
    void pre_sys_PostQuitMessage(int nExitCode) {
        using tFunc = void(WINAPI*)(int);
        SYS_CALL_VOID(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("PostQuitMessage"), nExitCode);
    }

    // 3. TrackMouseEvent - Can be in user32 or comctl32
    BOOL pre_sys_TrackMouseEvent(LPTRACKMOUSEEVENT lpEventTrack) {
        using tFunc = BOOL(WINAPI*)(LPTRACKMOUSEEVENT);
        SYS_CALL(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("TrackMouseEvent"), lpEventTrack);
    }

    // 1. SetCapture - Essential for ImGui clicks
    HWND pre_sys_SetCapture(HWND hWnd) {
        using tFunc = HWND(WINAPI*)(HWND);
        SYS_CALL_FORCE(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("SetCapture"), ecrypt("SetCapture"), hWnd);
    }

    // 2. ReleaseCapture - Essential for ImGui clicks
    BOOL pre_sys_ReleaseCapture() {
        using tFunc = BOOL(WINAPI*)(VOID);
        SYS_CALL_FORCE(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("ReleaseCapture"), ecrypt("ReleaseCapture"));
    }

    // 3. GetCapture
    HWND pre_sys_GetCapture() {
        using tFunc = HWND(WINAPI*)(VOID);
        SYS_CALL_FORCE(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("GetCapture"), ecrypt("GetCapture"));
    }

    // 7. GetMessageExtraInfo
    LPARAM pre_sys_GetMessageExtraInfo() {
        using tFunc = LPARAM(WINAPI*)(VOID);
        SYS_CALL(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("GetMessageExtraInfo"));
    }

    // 8. IsWindowUnicode
    BOOL pre_sys_IsWindowUnicode(HWND hWnd) {
        using tFunc = BOOL(WINAPI*)(HWND);
        SYS_CALL(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("IsWindowUnicode"), hWnd);
    }

    HWND pre_sys_SetFocus(HWND hWnd) {
        using tFunc = HWND(WINAPI*)(HWND);
        SYS_CALL(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("SetFocus"), hWnd);
    }

    int pre_sys_MessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType) {
        using tFunc = int(WINAPI*)(HWND, LPCSTR, LPCSTR, UINT);
        SYS_CALL(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("MessageBoxA"), hWnd, lpText, lpCaption, uType);
    }

    //fixed
    BOOL pre_sys_GetClassInfoExW(HINSTANCE hInstance, LPCWSTR lpszClass, LPWNDCLASSEXW lpwcx) {
        using tFunc = BOOL(WINAPI*)(HINSTANCE, LPCWSTR, LPWNDCLASSEXW);
        SYS_CALL_FORCE(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("GetClassInfoExW"), ecrypt("GetClassInfoExW"), hInstance, lpszClass, lpwcx);
    }
/*
    BOOL pre_sys_GetClassInfoExW(HINSTANCE hInstance, LPCWSTR lpszClass, LPWNDCLASSEXW lpwcx) {
        // Define the exact function signature
        using tFunc = BOOL(WINAPI*)(HINSTANCE, LPCWSTR, LPWNDCLASSEXW);

        // Execute the call through the obfuscated macro
        SYS_CALL(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("GetClassInfoExW"), hInstance, lpszClass, lpwcx);
    }
*/


    UINT pre_sys_MapVirtualKeyW(UINT uCode, UINT uMapType) {
        // Standard user32.dll call
        using tFunc = UINT(WINAPI*)(UINT, UINT);
        SYS_CALL(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("MapVirtualKeyW"), uCode, uMapType);
    }

    LONG pre_sys_GetWindowLongW(HWND hWnd, int nIndex) {
        // Requires ForceResolve due to common hooking/forwarding in Windows
        using tFunc = LONG(WINAPI*)(HWND, int);
        SYS_CALL_FORCE(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("GetWindowLongW"), ecrypt("GetWindowLongW"), hWnd, nIndex);
    }

    LRESULT pre_sys_DefWindowProcW(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
        // Requires ForceResolve to bypass mapper checks
        using tFunc = LRESULT(WINAPI*)(HWND, UINT, WPARAM, LPARAM);
        SYS_CALL_FORCE(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("DefWindowProcW"), ecrypt("DefWindowProcW"), hWnd, Msg, wParam, lParam);
    }

    BOOL pre_sys_PeekMessageW(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg) {
        // ForceResolve is recommended for message loop hooks
        using tFunc = BOOL(WINAPI*)(LPMSG, HWND, UINT, UINT, UINT);
        SYS_CALL_FORCE(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("PeekMessageW"), ecrypt("PeekMessageW"), lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
    }

    LRESULT pre_sys_DispatchMessageW(const MSG* lpMsg) {
        // ForceResolve is recommended for message loop hooks
        using tFunc = LRESULT(WINAPI*)(const MSG*);
        SYS_CALL_FORCE(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("DispatchMessageW"), ecrypt("DispatchMessageW"), lpMsg);
    }

    HGDIOBJ pre_sys_GetStockObject(int i) {
        // gdi32.dll is used here instead of user32.dll
        using tFunc = HGDIOBJ(WINAPI*)(int);
        SYS_CALL(tFunc, CT_HASHW(L"gdi32.dll"), ecrypt("gdi32.dll"), CT_HASH("GetStockObject"), i);
    }





    // ================= DWMAPI =================

    HRESULT pre_sys_DwmExtendFrameIntoClientArea(HWND hWnd, const MARGINS* pMarInset) {
        using tFunc = HRESULT(WINAPI*)(HWND, const MARGINS*);
        SYS_CALL(tFunc, H_DWMAPI, ecrypt("dwmapi.dll"), CT_HASH("DwmExtendFrameIntoClientArea"), hWnd, pMarInset);
    }

    NTSTATUS pre_sys_NtUserSendInput(UINT cInputs, INPUT* pInputs, int cbSize) {
        SYS_CALL(NTSTATUS(WINAPI*)(UINT, INPUT*, int), H_WIN32U, "win32u.dll", CT_HASH("NtUserSendInput"), cInputs, pInputs, cbSize);
    }

    BOOL pre_sys_NtUserSetCursorPos(int X, int Y) {
        SYS_CALL(BOOL(WINAPI*)(int, int), H_WIN32U, "win32u.dll", CT_HASH("NtUserSetCursorPos"), X, Y);
    }

    BOOL pre_sys_NtUserGetCursorPos(POINT* lpPoint) {
        SYS_CALL(BOOL(WINAPI*)(POINT*), H_WIN32U, "win32u.dll", CT_HASH("NtUserGetCursorPos"), lpPoint);
    }

    ATOM pre_sys_RegisterClassExA(const WNDCLASSEXA* lpwcx) {
        using tFunc = ATOM(WINAPI*)(const WNDCLASSEXA*);
        SYS_CALL(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("RegisterClassExA"), lpwcx);
    }

    BOOL pre_sys_UnregisterClassA(LPCSTR lpClassName, HINSTANCE hInstance) {
        using tFunc = BOOL(WINAPI*)(LPCSTR, HINSTANCE);
        SYS_CALL(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("UnregisterClassA"), lpClassName, hInstance);
    }

    //fixed
    HWND pre_sys_CreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) {
        using tFunc = HWND(WINAPI*)(DWORD, LPCSTR, LPCSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID);
        SYS_CALL_FORCE(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("CreateWindowExA"), ecrypt("CreateWindowExA"), dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
    }
/*
    HWND pre_sys_CreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle,int X,int Y,int nWidth,int nHeight,HWND hWndParent,HMENU hMenu,HINSTANCE hInstance,LPVOID lpParam) {
        using tFunc = HWND(WINAPI*)(DWORD, LPCSTR, LPCSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID);
        SYS_CALL(tFunc, H_USER32, ecrypt("user32.dll"), CT_HASH("CreateWindowExA"),dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
    }
*/
#ifdef __cplusplus
} // Закрываем extern "C"
#endif