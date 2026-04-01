#ifndef SCAPI_HPP
#define SCAPI_HPP

#include <Windows.h>
#include <winternl.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <d2d1.h>
#include <dwrite.h>
#include <imm.h>
#include <cstdint>
#include <intrin.h>

#include <tlhelp32.h>
#include <dwmapi.h>

// Define NTSTATUS if it was not included by winternl.h
#ifndef NTSTATUS
typedef LONG NTSTATUS;
#endif

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#endif

typedef UINT(WINAPI* tNtUserSendInput)(UINT cInputs, LPINPUT pInputs, int cbSize);

typedef struct _DYNAMIC_LIBRARY {
    HMODULE hModule;
    FARPROC functions[64];
} DYNAMIC_LIBRARY;

static HMODULE hKernel32 = NULL;
static HMODULE hUser32 = NULL;
static HMODULE hShell32 = NULL;
static HMODULE hD3D11 = NULL;
static HMODULE hD3DCompiler = NULL;
static HMODULE hImm32 = NULL;

typedef struct _UNICODE_STRING_T {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING_T;

typedef struct _LDR_DATA_TABLE_ENTRY_T {
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;
    PVOID DllBase;
    PVOID EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING_T FullDllName;
    UNICODE_STRING_T BaseDllName;
} LDR_DATA_TABLE_ENTRY_T, * PLDR_DATA_TABLE_ENTRY_T;

typedef struct _PEB_LDR_DATA_T {
    ULONG Length;
    BOOLEAN Initialized;
    PVOID SsHandle;
    LIST_ENTRY InLoadOrderModuleList;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
} PEB_LDR_DATA_T, * PPEB_LDR_DATA_T;

typedef struct _PEB_T {
    BOOLEAN InheritedAddressSpace;
    BOOLEAN ReadImageFileExecOptions;
    BOOLEAN BeingDebugged;
    union {
        BOOLEAN BitField;
        struct {
            BOOLEAN ImageUsesLargePages : 1;
            BOOLEAN IsProtectedProcess : 1;
            BOOLEAN IsImageDynamicallyRelocated : 1;
            BOOLEAN SkipPatchingUser32Forwarders : 1;
            BOOLEAN IsPackagedProcess : 1;
            BOOLEAN IsAppContainer : 1;
            BOOLEAN IsProtectedProcessLight : 1;
            BOOLEAN IsLongPathAwareProcess : 1;
        };
    };
    HANDLE Mutant;
    PVOID ImageBaseAddress;
    PPEB_LDR_DATA_T Ldr;
} PEB_T, * PPEB_T;

// =============================================================
// PART 2: LAZY IMPORTER ENGINE
// =============================================================

constexpr uint32_t val_32_const = 0x811c9dc5;
constexpr uint32_t prime_32_const = 0x1000193;

constexpr uint32_t HashString(const char* const str, const uint32_t value = val_32_const) noexcept {
    return (str[0] == '\0') ? value : HashString(&str[1], (value ^ uint32_t(str[0])) * prime_32_const);
}

constexpr uint32_t HashStringW(const wchar_t* const str, const uint32_t value = val_32_const) noexcept {
    return (str[0] == '\0') ? value : HashStringW(&str[1], (value ^ uint32_t(str[0])) * prime_32_const);
}

#define CT_HASH(str) ([]() { return HashString(str); }())
#define CT_HASHW(str) ([]() { return HashStringW(str); }())

__forceinline HMODULE GetModuleBase(uint32_t moduleHash) {
#ifdef _WIN64
    PPEB_T peb = (PPEB_T)__readgsqword(0x60);
#else
    PPEB_T peb = (PPEB_T)__readfsdword(0x30);
#endif

    PPEB_LDR_DATA_T ldr = peb->Ldr;
    PLIST_ENTRY head = &ldr->InMemoryOrderModuleList;
    PLIST_ENTRY curr = head->Flink;

    while (curr != head) {
        PLDR_DATA_TABLE_ENTRY_T entry = (PLDR_DATA_TABLE_ENTRY_T)((uint8_t*)curr - sizeof(LIST_ENTRY));

        if (entry->BaseDllName.Buffer) {
            uint32_t h = val_32_const;
            for (int i = 0; i < entry->BaseDllName.Length / 2; i++) {
                wchar_t c = entry->BaseDllName.Buffer[i];
                if (c >= 'A' && c <= 'Z') c += 32;
                h = (h ^ c) * prime_32_const;
            }
            if (h == moduleHash) return (HMODULE)entry->DllBase;
        }
        curr = curr->Flink;
    }
    return NULL;
}

__forceinline void* GetExportAddress(HMODULE hMod, uint32_t funcHash) {
    if (!hMod) return nullptr;

    uint8_t* base = (uint8_t*)hMod;
    PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)base;
    PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)(base + dos->e_lfanew);

    auto& exportDirInfo = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    uint32_t exportStart = exportDirInfo.VirtualAddress;
    uint32_t exportEnd = exportStart + exportDirInfo.Size;

    if (exportStart == 0) return nullptr;

    PIMAGE_EXPORT_DIRECTORY exp = (PIMAGE_EXPORT_DIRECTORY)(base + exportStart);
    uint32_t* names = (uint32_t*)(base + exp->AddressOfNames);
    uint16_t* ordinals = (uint16_t*)(base + exp->AddressOfNameOrdinals);
    uint32_t* functions = (uint32_t*)(base + exp->AddressOfFunctions);

    for (uint32_t i = 0; i < exp->NumberOfNames; ++i) {
        const char* name = (const char*)(base + names[i]);
        if (HashString(name) == funcHash) {
            uint32_t funcRva = functions[ordinals[i]];
            if (funcRva >= exportStart && funcRva < exportEnd) {
                return nullptr;
            }
            return (void*)(base + funcRva);
        }
    }
    return nullptr;
}

__forceinline void* GetSafeApi(uint32_t modHash, const char* modNameStr, uint32_t funcHash) {
    HMODULE hMod = GetModuleBase(modHash);

    if (!hMod) {
        HMODULE hKernel = GetModuleBase(CT_HASHW(L"kernel32.dll"));
        if (!hKernel) return nullptr;

        using tLoadLibraryA = HMODULE(WINAPI*)(LPCSTR);
        auto pLoadLib = (tLoadLibraryA)GetExportAddress(hKernel, CT_HASH("LoadLibraryA"));

        if (pLoadLib) {
            hMod = pLoadLib(modNameStr);
        }
    }

    if (!hMod) return nullptr;
    return GetExportAddress(hMod, funcHash);
}

// =============================================================
// RUNTIME CALL OBFUSCATION (Replace call_function for runtime)
// =============================================================
namespace sc_obf {
    // Structure to create a dummy array on the stack
    template <typename T, int N, int RealIdx>
    struct RuntimeCaller {
        __forceinline static T get(T real_ptr) {
            // Create an array of pointers on the stack (volatile so the compiler doesn't optimize it away)
            volatile uintptr_t arr[N];

            // Fill the array with junk (obfuscated addresses)
            for (int i = 0; i < N; i++) {
                // XOR with index creates different values that look like pointers
                arr[i] = (uintptr_t)real_ptr ^ (i * 0x55AA55AA);
            }

            // Place the real pointer at a pre-selected compile-time index
            arr[RealIdx] = (uintptr_t)real_ptr;

            // Use a volatile variable to access the index
            // This forces the processor to calculate the address at runtime, breaking static call graph analysis
            volatile int idx = RealIdx;

            return (T)arr[idx];
        }
    };
}

// Macro generates a unique index for each call using __COUNTER__
#define SC_OBF_CALL(ptr) sc_obf::RuntimeCaller<decltype(ptr), 10, (__COUNTER__ % 10)>::get(ptr)

// === MODIFIED MACROS ===

#define SYS_CALL(TYPE, MOD_HASH, MOD_STR, FUNC_HASH, ...) \
    static auto pFunc = (TYPE)nullptr; \
    if (!pFunc) { \
        pFunc = (TYPE)GetSafeApi(MOD_HASH, MOD_STR, FUNC_HASH); \
    } \
    /* Use our runtime call obfuscator */ \
    if (pFunc) return SC_OBF_CALL(pFunc)(__VA_ARGS__); \
    return {}; 

#define SYS_CALL_VOID(TYPE, MOD_HASH, MOD_STR, FUNC_HASH, ...) \
    static auto pFunc = (TYPE)nullptr; \
    if (!pFunc) { \
        pFunc = (TYPE)GetSafeApi(MOD_HASH, MOD_STR, FUNC_HASH); \
    } \
    if (pFunc) { SC_OBF_CALL(pFunc)(__VA_ARGS__); return; } \
    return;

#define H_KERNEL32 CT_HASHW(L"kernel32.dll")
#define H_USER32   CT_HASHW(L"user32.dll")
#define H_D3D11    CT_HASHW(L"d3d11.dll")
#define H_D3DCOMP  CT_HASHW(L"d3dcompiler_47.dll")
#define H_IMM32    CT_HASHW(L"imm32.dll")
#define H_SHELL32  CT_HASHW(L"shell32.dll")
#define H_WIN32U   CT_HASHW(L"win32u.dll")
#define H_ADVAPI32 CT_HASHW(L"advapi32.dll")
#define H_DWMAPI   CT_HASHW(L"dwmapi.dll")
#define H_NTDLL    CT_HASHW(L"ntdll.dll")
#define H_D2D1     CT_HASHW(L"d2d1.dll")
#define H_DWRITE   CT_HASHW(L"dwrite.dll")

#ifdef __cplusplus
extern "C" {
#endif
    BOOL pre_sys_GetThreadContext(HANDLE hThread, LPCONTEXT lpContext);
    HANDLE pre_sys_GetCurrentThread();

    BOOL pre_sys_QueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount);
    BOOL pre_sys_QueryPerformanceFrequency(LARGE_INTEGER* lpFrequency);
    HGLOBAL pre_sys_GlobalAlloc(UINT uFlags, SIZE_T dwBytes);
    HGLOBAL pre_sys_GlobalFree(HGLOBAL hMem);
    LPVOID pre_sys_GlobalLock(HGLOBAL hMem);
    BOOL pre_sys_GlobalUnlock(HGLOBAL hMem);
    int pre_sys_WideCharToMultiByte(UINT CodePage, DWORD dwFlags, LPCWCH lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte, LPCCH lpDefaultChar, LPBOOL lpUsedDefaultChar);
    BOOL pre_sys_SetConsoleTitleA(LPCSTR lpConsoleTitle);
    HANDLE pre_sys_GetStdHandle(DWORD nStdHandle);
    BOOL pre_sys_SetConsoleMode(HANDLE hConsoleHandle, DWORD dwMode);
    BOOL pre_sys_GetConsoleMode(HANDLE hConsoleHandle, LPDWORD lpMode);
    BOOL pre_sys_GetConsoleCursorInfo(HANDLE hConsoleOutput, PCONSOLE_CURSOR_INFO lpConsoleCursorInfo);
    BOOL pre_sys_SetConsoleCursorInfo(HANDLE hConsoleOutput, const CONSOLE_CURSOR_INFO* lpConsoleCursorInfo);
    BOOL pre_sys_SetCurrentConsoleFontEx(HANDLE hConsoleOutput, BOOL bMaximumWindow, PCONSOLE_FONT_INFOEX lpConsoleCurrentFontEx);
    BOOL pre_sys_GetCurrentConsoleFontEx(HANDLE hConsoleOutput, BOOL bMaximumWindow, PCONSOLE_FONT_INFOEX lpConsoleCurrentFontEx);
    BOOL pre_sys_FlushConsoleInputBuffer(HANDLE hConsoleInput);
    int pre_sys_GetLocaleInfoA(LCID Locale, LCTYPE LCType, LPSTR lpLCData, int cchData);
    VOID pre_sys_GetSystemTimeAsFileTime(LPFILETIME lpSystemTimeAsFileTime);
    DWORD pre_sys_GetCurrentProcessId(void);
    DWORD pre_sys_GetCurrentThreadId(void);
    HMODULE pre_sys_GetModuleHandleW(LPCWSTR lpModuleName);
    LPTOP_LEVEL_EXCEPTION_FILTER pre_sys_SetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter);
    VOID pre_sys_WakeAllConditionVariable(PCONDITION_VARIABLE ConditionVariable);
    VOID pre_sys_Sleep(DWORD dwMilliseconds);
    BOOL pre_sys_SleepConditionVariableSRW(PCONDITION_VARIABLE ConditionVariable, PSRWLOCK SRWLock, DWORD dwMilliseconds, ULONG Flags);
    VOID pre_sys_AcquireSRWLockExclusive(PSRWLOCK SRWLock);
    VOID pre_sys_ReleaseSRWLockExclusive(PSRWLOCK SRWLock);
    BOOL pre_sys_FreeLibrary(HMODULE hLibModule);
    FARPROC pre_sys_GetProcAddress(HMODULE hModule, LPCSTR lpProcName);
    HMODULE pre_sys_LoadLibraryA(LPCSTR lpLibFileName);
    VOID pre_sys_InitializeSListHead(PSLIST_HEADER ListHead);
    DWORD pre_sys_GetFullPathNameA(LPCSTR lpFileName, DWORD nBufferLength, LPSTR lpBuffer, LPSTR* lpFilePart);
    DWORD pre_sys_GetModuleFileNameA(HMODULE hModule, LPSTR lpFilename, DWORD nSize);
    int pre_sys_MultiByteToWideChar(UINT CodePage, DWORD dwFlags, LPCCH lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar);
    HWND pre_sys_GetConsoleWindow(void);
    UINT pre_sys_timeBeginPeriod(UINT uPeriod);

    LONG pre_sys_GetWindowLongA(HWND hWnd, int nIndex);
    BOOL pre_sys_SetLayeredWindowAttributes(HWND hwnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);
    HCURSOR pre_sys_LoadCursorA(HINSTANCE hInstance, LPCSTR lpCursorName);
    BOOL pre_sys_ClientToScreen(HWND hWnd, LPPOINT lpPoint);
    int pre_sys_GetSystemMetrics(int nIndex);
    BOOL pre_sys_IsWindow(HWND hWnd);
    BOOL pre_sys_ScreenToClient(HWND hWnd, LPPOINT lpPoint);

    BOOL pre_sys_GetClientRect(HWND hWnd, LPRECT lpRect);
    HINSTANCE pre_sys_ShellExecuteW(HWND hwnd, LPCWSTR lpOperation, LPCWSTR lpFile, LPCWSTR lpParameters, LPCWSTR lpDirectory, INT nShowCmd);
    HRESULT pre_sys_D3D11CreateDeviceAndSwapChain(IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, const D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, IDXGISwapChain** ppSwapChain, ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext);
    HRESULT pre_sys_D3DCompile(LPCVOID pSrcData, SIZE_T SrcDataSize, LPCSTR pSourceName, const D3D_SHADER_MACRO* pDefines, ID3DInclude* pInclude, LPCSTR pEntrypoint, LPCSTR pTarget, UINT Flags1, UINT Flags2, ID3DBlob** ppCode, ID3DBlob** ppErrorMsgs);
    BOOL pre_sys_ImmSetCompositionWindow(HIMC hIMC, LPCOMPOSITIONFORM lpCompForm);
    BOOL pre_sys_ImmSetCandidateWindow(HIMC hIMC, LPCANDIDATEFORM lpCandidate);
    HIMC pre_sys_ImmGetContext(HWND hWnd);
    BOOL pre_sys_ImmReleaseContext(HWND hWnd, HIMC hIMC);

    HHOOK pre_sys_SetWindowsHookExA(int idHook, HOOKPROC lpfn, HINSTANCE hMod, DWORD dwThreadId);
    BOOL pre_sys_ShowWindow(HWND hWnd, int nCmdShow);
    LONG pre_sys_SetWindowLongA(HWND hWnd, int nIndex, LONG dwNewLong);
    LONG pre_sys_SetWindowLongW(HWND hWnd, int nIndex, LONG dwNewLong);
    HWND pre_sys_FindWindowA(LPCSTR lpClassName, LPCSTR lpWindowName);
    HWND pre_sys_FindWindowExA(HWND hWndParent, HWND hWndChildAfter, LPCSTR lpszClass, LPCSTR lpszWindow);
    BOOL pre_sys_IsWindowVisible(HWND hWnd);
    HWND pre_sys_GetForegroundWindow(void);
    SHORT pre_sys_GetAsyncKeyState(int vKey);
    SHORT pre_sys_GetKeyState(int nVirtKey);
    BOOL pre_sys_GetKeyboardState(PBYTE lpKeyState);
    int pre_sys_ToUnicode(UINT wVirtKey, UINT wScanCode, const BYTE* lpKeyState, LPWSTR pwszBuff, int cchBuff, UINT wFlags);
    HKL pre_sys_GetKeyboardLayout(DWORD idThread);
    BOOL pre_sys_SetCursorPos(int X, int Y);
    BOOL pre_sys_GetCursorPos(LPPOINT lpPoint);
    HCURSOR pre_sys_SetCursor(HCURSOR hCursor);
    int pre_sys_ShowCursor(BOOL bShow);
    BOOL pre_sys_PeekMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);
    LRESULT pre_sys_DispatchMessageA(const MSG* lpMsg);
    BOOL pre_sys_TranslateMessage(const MSG* lpMsg);
    LRESULT pre_sys_CallNextHookEx(HHOOK hhk, int nCode, WPARAM wParam, LPARAM lParam);
    BOOL pre_sys_UnhookWindowsHookEx(HHOOK hhk);
    BOOL pre_sys_OpenClipboard(HWND hWndNewOwner);
    BOOL pre_sys_CloseClipboard(void);
    BOOL pre_sys_EmptyClipboard(void);
    HANDLE pre_sys_GetClipboardData(UINT uFormat);
    HANDLE pre_sys_SetClipboardData(UINT uFormat, HANDLE hMem);
    HANDLE pre_sys_SWDA(HWND hWnd, DWORD dwAffinity);

    BOOL pre_sys_OpenProcessToken(HANDLE ProcessHandle, DWORD DesiredAccess, PHANDLE TokenHandle);
    BOOL pre_sys_GetTokenInformation(HANDLE TokenHandle, TOKEN_INFORMATION_CLASS TokenInformationClass, LPVOID TokenInformation, DWORD TokenInformationLength, PDWORD ReturnLength);
    BOOL pre_sys_LookupPrivilegeValueA(LPCSTR lpSystemName, LPCSTR lpName, PLUID lpLuid);
    HANDLE pre_sys_CreateToolhelp32Snapshot(DWORD dwFlags, DWORD th32ProcessID);
    BOOL pre_sys_Process32First(HANDLE hSnapshot, LPPROCESSENTRY32 lppe);
    BOOL pre_sys_Process32Next(HANDLE hSnapshot, LPPROCESSENTRY32 lppe);
    HANDLE pre_sys_OpenProcess(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwProcessId);
    BOOL pre_sys_PrivilegeCheck(HANDLE ClientToken, PPRIVILEGE_SET RequiredPrivileges, LPBOOL pfResult);
    BOOL pre_sys_DuplicateTokenEx(HANDLE hExistingToken, DWORD dwDesiredAccess, LPSECURITY_ATTRIBUTES lpTokenAttributes, SECURITY_IMPERSONATION_LEVEL ImpersonationLevel, TOKEN_TYPE TokenType, PHANDLE phNewToken);
    BOOL pre_sys_SetThreadToken(PHANDLE Thread, HANDLE Token);
    BOOL pre_sys_SetTokenInformation(HANDLE TokenHandle, TOKEN_INFORMATION_CLASS TokenInformationClass, LPVOID TokenInformation, DWORD TokenInformationLength);
    BOOL pre_sys_RevertToSelf(void);
    VOID pre_sys_GetStartupInfoW(LPSTARTUPINFOW lpStartupInfo);
    LPSTR pre_sys_GetCommandLineA(void);
    BOOL pre_sys_CreateProcessAsUserA(HANDLE hToken, LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCSTR lpCurrentDirectory, LPSTARTUPINFOA lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);
    VOID pre_sys_ExitProcess(UINT uExitCode);
    BOOL pre_sys_CloseHandle(HANDLE hObject);
    DWORD pre_sys_GetLastError(void);

    ATOM pre_sys_RegisterClassExW(CONST WNDCLASSEXW* Arg1);
    BOOL pre_sys_UnregisterClassW(LPCWSTR lpClassName, HINSTANCE hInstance);
    HWND pre_sys_CreateWindowExW(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
    BOOL pre_sys_GetWindowRect(HWND hWnd, LPRECT lpRect);
    BOOL pre_sys_UpdateWindow(HWND hWnd);
    LONG_PTR pre_sys_GetWindowLongPtrA(HWND hWnd, int nIndex);
    LONG_PTR pre_sys_SetWindowLongPtrA(HWND hWnd, int nIndex, LONG_PTR dwNewLong);
    BOOL pre_sys_DestroyWindow(HWND hWnd);
    HWND pre_sys_SetActiveWindow(HWND hWnd);
    BOOL pre_sys_SetForegroundWindow(HWND hWnd);

    HRESULT pre_sys_DwmExtendFrameIntoClientArea(HWND hWnd, const MARGINS* pMarInset);

    HANDLE pre_sys_GetCurrentProcess(void);
    BOOL pre_sys_SetPriorityClass(HANDLE hProcess, DWORD dwPriorityClass);

    LRESULT pre_sys_DefWindowProcW(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
    LRESULT pre_sys_DefWindowProcA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
    void pre_sys_PostQuitMessage(int nExitCode);
    BOOL pre_sys_TrackMouseEvent(LPTRACKMOUSEEVENT lpEventTrack);
    BOOL pre_sys_IsWindowUnicode(HWND hWnd);
    BOOL pre_sys_ReleaseCapture();
    HWND pre_sys_SetCapture(HWND hWnd);
    LPARAM pre_sys_GetMessageExtraInfo();
    HWND pre_sys_GetCapture();
    HWND pre_sys_SetFocus(HWND hWnd);
    int pre_sys_MessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);
    BOOL pre_sys_GetClassInfoExW(HINSTANCE hInstance, LPCWSTR lpszClass, LPWNDCLASSEXW lpwcx);

    UINT pre_sys_MapVirtualKeyW(UINT uCode, UINT uMapType);
    LONG pre_sys_GetWindowLongW(HWND hWnd, int nIndex);
    BOOL pre_sys_PeekMessageW(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);
    LRESULT pre_sys_DispatchMessageW(const MSG* lpMsg);
    HGDIOBJ pre_sys_GetStockObject(int i);

    __forceinline void pre_sys_YieldProcessor() {
        _mm_pause();
    }

    NTSTATUS pre_sys_NtUserSendInput(UINT cInputs, INPUT* pInputs, int cbSize);
    BOOL pre_sys_NtUserSetCursorPos(int X, int Y);
    BOOL pre_sys_NtUserGetCursorPos(POINT* lpPoint);

    ATOM pre_sys_RegisterClassExA(const WNDCLASSEXA* lpwcx);
    BOOL pre_sys_UnregisterClassA(LPCSTR lpClassName, HINSTANCE hInstance);
    HWND pre_sys_CreateWindowExA(
        DWORD dwExStyle,
        LPCSTR lpClassName,
        LPCSTR lpWindowName,
        DWORD dwStyle,
        int X,
        int Y,
        int nWidth,
        int nHeight,
        HWND hWndParent,
        HMENU hMenu,
        HINSTANCE hInstance,
        LPVOID lpParam
    );

    HRESULT pre_sys_D2D1CreateFactory_Raw(
        D2D1_FACTORY_TYPE factoryType,
        REFIID riid,
        const D2D1_FACTORY_OPTIONS* pFactoryOptions,
        void** ppIFactory
    );

    HRESULT pre_sys_DWriteCreateFactory_Raw(
        DWRITE_FACTORY_TYPE factoryType,
        REFIID iid,
        IUnknown** factory
    );
#ifdef __cplusplus
}
#endif

// =============================================================
// C++ Templates
// =============================================================
#ifdef __cplusplus

template<class Factory>
HRESULT pre_sys_D2D1CreateFactory(D2D1_FACTORY_TYPE factoryType, Factory** factory) {
    return pre_sys_D2D1CreateFactory_Raw(
        factoryType,
        __uuidof(Factory),
        nullptr,
        reinterpret_cast<void**>(factory)
    );
}

template<class Factory>
HRESULT pre_sys_D2D1CreateFactory(
    D2D1_FACTORY_TYPE factoryType,
    const D2D1_FACTORY_OPTIONS& factoryOptions,
    Factory** factory
) {
    return pre_sys_D2D1CreateFactory_Raw(
        factoryType,
        __uuidof(Factory),
        &factoryOptions,
        reinterpret_cast<void**>(factory)
    );
}

template <class Factory>
HRESULT pre_sys_DWriteCreateFactory(DWRITE_FACTORY_TYPE factoryType, Factory** factory) {
    return pre_sys_DWriteCreateFactory_Raw(
        factoryType,
        __uuidof(Factory),
        reinterpret_cast<IUnknown**>(factory)
    );
}

#endif // __cplusplus

#endif // SCAPI_HPP