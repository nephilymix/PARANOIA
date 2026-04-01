#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <iostream>

#define PING_MAGIC 0xcb76d8cef132

#define CTL_CODE( DeviceType, Function, Method, Access ) ( ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) )
#define FILE_DEVICE_UNKNOWN 0x00000022
#define METHOD_BUFFERED 0
#define FILE_SPECIAL_ACCESS 0

#define IO_PING            CTL_CODE(FILE_DEVICE_UNKNOWN, 0x900, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define IO_GET_CLIENT_BASE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x901, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define IO_READ_MEM        CTL_CODE(FILE_DEVICE_UNKNOWN, 0x902, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define IO_WRITE_MEM       CTL_CODE(FILE_DEVICE_UNKNOWN, 0x903, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define IO_ALLOC_MEM       CTL_CODE(FILE_DEVICE_UNKNOWN, 0x904, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define IO_PROTECT_MEM     CTL_CODE(FILE_DEVICE_UNKNOWN, 0x905, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define IO_FREE_MEM        CTL_CODE(FILE_DEVICE_UNKNOWN, 0x906, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define IO_APC_INJECT      CTL_CODE(FILE_DEVICE_UNKNOWN, 0x907, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define IO_MOUSE_EVENT     CTL_CODE(FILE_DEVICE_UNKNOWN, 0x908, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define IO_SET_PROTECTED_PID CTL_CODE(FILE_DEVICE_UNKNOWN, 0x909, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

#define MOUSE_LEFT_BUTTON_DOWN   0x0001
#define MOUSE_LEFT_BUTTON_UP     0x0002
#define MOUSE_RIGHT_BUTTON_DOWN  0x0004
#define MOUSE_RIGHT_BUTTON_UP    0x0008

struct MouseRequest {
    LONG DeltaX;
    LONG DeltaY;
    USHORT ButtonFlags;
};

struct DriverRequest {
    ULONG ProcessId;
    ULONG ThreadId;
    ULONGLONG TargetAddress;
    ULONGLONG BufferAddress;
    SIZE_T Size;
    ULONGLONG Output;
    ULONG Protection;
    ULONG AllocationType;
    WCHAR ModuleName[260];
    MouseRequest Mouse;
};

namespace NtDefs {
    typedef long NTSTATUS;
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#define NT_SUCCESS(Status) (((NtDefs::NTSTATUS)(Status)) >= 0)
    typedef struct _UNICODE_STRING {
        USHORT Length;
        USHORT MaximumLength;
        PWSTR  Buffer;
    } UNICODE_STRING, * PUNICODE_STRING;

    typedef struct _OBJECT_ATTRIBUTES {
        ULONG Length;
        HANDLE RootDirectory;
        PUNICODE_STRING ObjectName;
        ULONG Attributes;
        PVOID SecurityDescriptor;
        PVOID SecurityQualityOfService;
    } OBJECT_ATTRIBUTES, * POBJECT_ATTRIBUTES;

    typedef struct _IO_STATUS_BLOCK {
        union {
            NTSTATUS Status;
            PVOID Pointer;
        };
        ULONG_PTR Information;
    } IO_STATUS_BLOCK, * PIO_STATUS_BLOCK;

    // ИСПРАВЛЕНО: Добавлен *POBJECT_DIRECTORY_INFORMATION
    typedef struct _OBJECT_DIRECTORY_INFORMATION {
        UNICODE_STRING Name;
        UNICODE_STRING TypeName;
    } OBJECT_DIRECTORY_INFORMATION, * POBJECT_DIRECTORY_INFORMATION;

    typedef enum _KWAIT_REASON {
        Executive, FreePage, PageIn, PoolAllocation, DelayExecution,
        Suspended, UserRequest, WrExecutive, WrFreePage, WrPageIn,
        WrPoolAllocation, WrDelayExecution, WrSuspended, WrUserRequest,
        EventPair, CxxTp, WrEventPair, WrCxxTp, MaximumWaitReason
    } KWAIT_REASON;

    typedef struct _CLIENT_ID {
        HANDLE UniqueProcess;
        HANDLE UniqueThread;
    } CLIENT_ID;

    typedef struct _SYSTEM_THREAD_INFORMATION {
        LARGE_INTEGER KernelTime;
        LARGE_INTEGER UserTime;
        LARGE_INTEGER CreateTime;
        ULONG WaitTime;
        PVOID StartAddress;
        CLIENT_ID ClientId;
        LONG Priority;
        LONG BasePriority;
        ULONG ContextSwitches;
        ULONG ThreadState;
        KWAIT_REASON WaitReason;
    } SYSTEM_THREAD_INFORMATION, * PSYSTEM_THREAD_INFORMATION;

    // ПОЛНАЯ СТРУКТУРА (БЕЗ СОКРАЩЕНИЙ)
    typedef struct _SYSTEM_PROCESS_INFORMATION {
        ULONG NextEntryOffset;
        ULONG NumberOfThreads;
        LARGE_INTEGER WorkingSetPrivateSize;
        ULONG HardFaultCount;
        ULONG NumberOfThreadsHighWatermark;
        ULONGLONG CycleTime;
        LARGE_INTEGER CreateTime;
        LARGE_INTEGER UserTime;
        LARGE_INTEGER KernelTime;
        UNICODE_STRING ImageName;
        LONG BasePriority;
        HANDLE UniqueProcessId;
        HANDLE InheritedFromUniqueProcessId;
        ULONG HandleCount;
        ULONG SessionId;
        ULONG_PTR UniqueProcessKey;
        SIZE_T PeakVirtualSize;
        SIZE_T VirtualSize;
        ULONG PageFaultCount;
        SIZE_T PeakWorkingSetSize;
        SIZE_T WorkingSetSize;
        SIZE_T QuotaPeakPagedPoolUsage;
        SIZE_T QuotaPagedPoolUsage;
        SIZE_T QuotaPeakNonPagedPoolUsage;
        SIZE_T QuotaNonPagedPoolUsage;
        SIZE_T PagefileUsage;
        SIZE_T PeakPagefileUsage;
        SIZE_T PrivatePageCount;
        LARGE_INTEGER ReadOperationCount;
        LARGE_INTEGER WriteOperationCount;
        LARGE_INTEGER OtherOperationCount;
        LARGE_INTEGER ReadTransferCount;
        LARGE_INTEGER WriteTransferCount;
        LARGE_INTEGER OtherTransferCount;

        // Массив потоков идет здесь
        SYSTEM_THREAD_INFORMATION Threads[1];
    } SYSTEM_PROCESS_INFORMATION, * PSYSTEM_PROCESS_INFORMATION;

    typedef enum _FILE_INFORMATION_CLASS {
        FileDispositionInformation = 13
    } FILE_INFORMATION_CLASS;

    // 2. Define the structure required for deletion
    typedef struct _FILE_DISPOSITION_INFORMATION {
        BOOLEAN DeleteFile;
    } FILE_DISPOSITION_INFORMATION, * PFILE_DISPOSITION_INFORMATION;
}

// Структура для удаления файла
typedef struct _FILE_DISPOSITION_INFORMATION {
    BOOLEAN DeleteFile;
} FILE_DISPOSITION_INFORMATION, * PFILE_DISPOSITION_INFORMATION;

// Класс информации для NtSetInformationFile
//#define FileDispositionInformation 13

// --- КЛАСС ДЛЯ РАБОТЫ С ПАМЯТЬЮ ---

class __scrt_km {
private:
    HANDLE hDriver = INVALID_HANDLE_VALUE;
    DWORD targetPid = 0;

    // Указатели на функции NTAPI
    typedef NtDefs::NTSTATUS(__stdcall* f_NtOpenDirectoryObject)(PHANDLE, ULONG, NtDefs::POBJECT_ATTRIBUTES);
    typedef NtDefs::NTSTATUS(__stdcall* f_NtQueryDirectoryObject)(HANDLE, PVOID, ULONG, BOOLEAN, BOOLEAN, PULONG, PULONG);
    typedef NtDefs::NTSTATUS(__stdcall* f_NtOpenFile)(PHANDLE, ULONG, NtDefs::POBJECT_ATTRIBUTES, NtDefs::PIO_STATUS_BLOCK, ULONG, ULONG);
    typedef NtDefs::NTSTATUS(__stdcall* f_NtDeviceIoControlFile)(HANDLE, PVOID, PVOID, PVOID, NtDefs::PIO_STATUS_BLOCK, ULONG, PVOID, ULONG, PVOID, ULONG);
    typedef NtDefs::NTSTATUS(__stdcall* f_NtClose)(HANDLE);
    typedef VOID(__stdcall* f_RtlInitUnicodeString)(NtDefs::PUNICODE_STRING, PCWSTR);
    typedef NtDefs::NTSTATUS(__stdcall* f_NtQuerySystemInformation)(ULONG, PVOID, ULONG, PULONG);
    typedef NtDefs::NTSTATUS(__stdcall* f_NtReadFile)(HANDLE, HANDLE, PVOID, PVOID, NtDefs::PIO_STATUS_BLOCK, PVOID, ULONG, PULONG, PULONG);
    typedef NtDefs::NTSTATUS(__stdcall* f_NtSetInformationFile)(HANDLE, NtDefs::PIO_STATUS_BLOCK, PVOID, ULONG, ULONG);

    f_NtOpenDirectoryObject NtOpenDirectoryObject = nullptr;
    f_NtQueryDirectoryObject NtQueryDirectoryObject = nullptr;
    f_NtOpenFile NtOpenFile = nullptr;
    f_NtDeviceIoControlFile NtDeviceIoControlFile = nullptr;
    f_NtClose NtClose = nullptr;
    f_RtlInitUnicodeString RtlInitUnicodeString = nullptr;
    f_NtQuerySystemInformation NtQuerySystemInformation = nullptr;
    f_NtReadFile NtReadFile = nullptr;
    f_NtSetInformationFile NtSetInformationFile = nullptr;

    bool __scrt_intf() {
        //TEST         HMODULE hNtdll = LoadLibraryA("ntdll.dll");
        HMODULE hNtdll = pre_sys_LoadLibraryA("ntdll.dll");
        if (!hNtdll) return false;

        NtOpenDirectoryObject = (f_NtOpenDirectoryObject)GetProcAddress(hNtdll, ecrypt("NtOpenDirectoryObject"));
        NtQueryDirectoryObject = (f_NtQueryDirectoryObject)GetProcAddress(hNtdll, ecrypt("NtQueryDirectoryObject"));
        NtOpenFile = (f_NtOpenFile)GetProcAddress(hNtdll, ecrypt("NtOpenFile"));
        NtDeviceIoControlFile = (f_NtDeviceIoControlFile)GetProcAddress(hNtdll, ecrypt("NtDeviceIoControlFile"));
        NtClose = (f_NtClose)GetProcAddress(hNtdll, ecrypt("NtClose"));
        RtlInitUnicodeString = (f_RtlInitUnicodeString)GetProcAddress(hNtdll, ecrypt("RtlInitUnicodeString"));
        NtQuerySystemInformation = (f_NtQuerySystemInformation)GetProcAddress(hNtdll, ecrypt("NtQuerySystemInformation"));
        NtReadFile = (f_NtReadFile)GetProcAddress(hNtdll, ecrypt("NtReadFile"));
        NtSetInformationFile = (f_NtSetInformationFile)GetProcAddress(hNtdll, "NtSetInformationFile");

        return NtOpenDirectoryObject && NtDeviceIoControlFile && NtReadFile && NtOpenFile && NtDeviceIoControlFile && NtSetInformationFile;
    }

    /*
       // --- ОБНОВЛЕННАЯ ФУНКЦИЯ ЧТЕНИЯ GUID С ЛОГАМИ ---
       std::wstring __read_guid() {
           //std::wcout << L"[DEBUG] __read_guid: start." << std::endl;

           if (!NtOpenFile || !NtReadFile || !NtClose || !RtlInitUnicodeString) {
               //std::wcout << L"[ERROR] __read_guid: NTAPI functions not loaded!" << std::endl;
               return L"";
           }

           // Путь к файлу
           NtDefs::UNICODE_STRING uFileName;
           RtlInitUnicodeString(&uFileName, ecrypt(L"\\??\\C:\\log.txt"));

           //std::wcout << L"[DEBUG] __read_guid: Trying to open " << uFileName.Buffer << std::endl;

           NtDefs::OBJECT_ATTRIBUTES objAttr = {
               sizeof(NtDefs::OBJECT_ATTRIBUTES),
               NULL,
               &uFileName,
               0x00000040, // OBJ_CASE_INSENSITIVE
               NULL,
               NULL
           };

           NtDefs::IO_STATUS_BLOCK ioStatus;
           HANDLE hFile = NULL;

           // Открываем файл
           NtDefs::NTSTATUS status = NtOpenFile(&hFile,
               0x80100000, // GENERIC_READ | SYNCHRONIZE
               &objAttr,
               &ioStatus,
               0x00000001, // FILE_SHARE_READ
               0x00000020  // FILE_SYNCHRONOUS_IO_NONALERT
           );

           if (status < 0 || hFile == NULL) {
               //std::wcout << L"[ERROR] NtOpenFile failed. Status: 0x" << std::hex << status << std::dec << std::endl;
               return L"";
           }

           //std::wcout << L"[DEBUG] NtOpenFile success. Handle: " << hFile << std::endl;

           wchar_t buffer[128] = { 0 };

           // Читаем файл
           status = NtReadFile(hFile, NULL, NULL, NULL, &ioStatus, buffer, sizeof(buffer) - 2, NULL, NULL);

           NtClose(hFile);

           if (status < 0) {
               //std::wcout << L"[ERROR] NtReadFile failed. Status: 0x" << std::hex << status << std::dec << std::endl;
               return L"";
           }

           //std::wcout << L"[DEBUG] NtReadFile success. Bytes read: " << ioStatus.Information << std::endl;

           std::wstring guid(buffer);
           //std::wcout << L"[DEBUG] Raw GUID from file: [" << guid << L"]" << std::endl;

           // Чистим мусор
           while (!guid.empty() && (guid.back() == L'\0' || guid.back() == L'\r' || guid.back() == L'\n' || guid.back() == L' ')) {
               guid.pop_back();
           }

           //std::wcout << L"[DEBUG] Cleaned GUID: [" << guid << L"]" << std::endl;
           return guid;
       }

       void __delete_file(const std::wstring& ntPath) {
           if (!NtOpenFile || !NtSetInformationFile || !NtClose || !RtlInitUnicodeString) return;

           NtDefs::UNICODE_STRING uName;
           RtlInitUnicodeString(&uName, ntPath.c_str());

           NtDefs::OBJECT_ATTRIBUTES objAttr = {
               sizeof(NtDefs::OBJECT_ATTRIBUTES),
               NULL,
               &uName,
               0x00000040, // OBJ_CASE_INSENSITIVE
               NULL,
               NULL
           };

           NtDefs::IO_STATUS_BLOCK ioStatus;
           HANDLE hFile = NULL;

           // 1. Открываем файл с правом DELETE (0x00010000)
           // FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT = 0x60
           NtDefs::NTSTATUS status = NtOpenFile(&hFile,
               0x00010000 | 0x80000000, // DELETE | SYNCHRONIZE (для корректности)
               &objAttr,
               &ioStatus,
               0x00000007, // FILE_SHARE_ALL (чтобы открыть, даже если кто-то читает)
               0x00000060  // FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT
           );

           if (status >= 0 && hFile) {
               // 2. Помечаем файл на удаление
               FILE_DISPOSITION_INFORMATION dispInfo;
               dispInfo.DeleteFile = TRUE;

               status = NtSetInformationFile(hFile, &ioStatus, &dispInfo, sizeof(dispInfo), FileDispositionInformation);

               // 3. Закрытие хендла триггерит удаление
               NtClose(hFile);

               // if (status >= 0) std::wcout << L"[DEBUG] File deleted: " << ntPath << std::endl;
           }
       }

    */

    // Helper function to delete file via NTAPI
    void __delete_file(const std::wstring& ntPath) {
        // Check if critical functions are loaded
        if (!NtOpenFile || !NtSetInformationFile || !NtClose || !RtlInitUnicodeString) return;

        NtDefs::UNICODE_STRING uName;
        RtlInitUnicodeString(&uName, ntPath.c_str());

        NtDefs::OBJECT_ATTRIBUTES objAttr = {
            sizeof(NtDefs::OBJECT_ATTRIBUTES),
            NULL,
            &uName,
            0x00000040, // OBJ_CASE_INSENSITIVE
            NULL,
            NULL
        };

        NtDefs::IO_STATUS_BLOCK ioStatus;
        HANDLE hFile = NULL;

        // 1. Open file with DELETE access
        // FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT = 0x60
        NtDefs::NTSTATUS status = NtOpenFile(&hFile,
            0x00010000 | 0x80000000, // DELETE | SYNCHRONIZE
            &objAttr,
            &ioStatus,
            0x00000007, // FILE_SHARE_ALL (allow shared access just in case)
            0x00000060  // FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT
        );

        if (status >= 0 && hFile) {
            // 2. Set file disposition to delete
            NtDefs::FILE_DISPOSITION_INFORMATION dispInfo;
            dispInfo.DeleteFile = TRUE;

            status = NtSetInformationFile(hFile, &ioStatus, &dispInfo, sizeof(dispInfo), NtDefs::FileDispositionInformation);

            // 3. Closing the handle triggers the actual deletion
            NtClose(hFile);
        }
    }

    std::wstring __read_guid() {
        // Check if critical functions are loaded
        if (!NtOpenFile || !NtReadFile || !NtClose || !RtlInitUnicodeString) {
            return L"";
        }

        // Prepare the file path explicitly to reuse it for deletion later
        // NOTE: Using std::wstring to ensure the buffer stays valid
        std::wstring sNtPath = ecrypt(L"\\??\\C:\\system.txt");

        NtDefs::UNICODE_STRING uFileName;
        RtlInitUnicodeString(&uFileName, sNtPath.c_str());

        NtDefs::OBJECT_ATTRIBUTES objAttr = {
            sizeof(NtDefs::OBJECT_ATTRIBUTES),
            NULL,
            &uFileName,
            0x00000040, // OBJ_CASE_INSENSITIVE
            NULL,
            NULL
        };

        NtDefs::IO_STATUS_BLOCK ioStatus;
        HANDLE hFile = NULL;

        // Open file for reading
        NtDefs::NTSTATUS status = NtOpenFile(&hFile,
            0x80100000, // GENERIC_READ | SYNCHRONIZE
            &objAttr,
            &ioStatus,
            0x00000001, // FILE_SHARE_READ
            0x00000020  // FILE_SYNCHRONOUS_IO_NONALERT
        );

        if (status < 0 || hFile == NULL) {
            return L"";
        }

        wchar_t buffer[128] = { 0 };

        // Read file content
        status = NtReadFile(hFile, NULL, NULL, NULL, &ioStatus, buffer, sizeof(buffer) - 2, NULL, NULL);

        // CRITICAL: Close the read handle before attempting to delete
        NtClose(hFile);

        // If read was successful (or even if it failed, depending on logic), delete the file
        // We attempt to delete the file regardless of read success to clean up traces
        if (status >= 0)__delete_file(sNtPath);

        if (status < 0) {
            return L"";
        }

        std::wstring guid(buffer);

        // Clean up whitespace and non-printable characters
        while (!guid.empty() && (guid.back() == L'\0' || guid.back() == L'\r' || guid.back() == L'\n' || guid.back() == L' ')) {
            guid.pop_back();
        }

        return guid;
    }

    HANDLE __scrt_sfd() {
        //std::wcout << L"[DEBUG] __scrt_sfd: start." << std::endl;

        if (!NtOpenFile) {
            //std::wcout << L"[ERROR] NtOpenFile address is missing!" << std::endl;
            return INVALID_HANDLE_VALUE;
        }

        // 1. Читаем GUID
        std::wstring guid = __read_guid();

        if (guid.empty()) {
            //std::wcout << L"[ERROR] __scrt_sfd: GUID is empty. Aborting." << std::endl;
            return INVALID_HANDLE_VALUE;
        }

        // Валидация
        if (guid.length() < 38 || guid[0] != L'{') {
            //std::wcout << L"[ERROR] Invalid GUID format: " << guid << std::endl;
            return INVALID_HANDLE_VALUE;
        }

        // 2. Формируем путь
        // ВАЖНО: Убедись, что ecrypt корректно разворачивается или используй просто строку L"\\Device\\"
        std::wstring fullPath = ecrypt(L"\\Device\\") + guid;
        //std::wcout << L"[DEBUG] Target device path: " << fullPath << std::endl;

        NtDefs::UNICODE_STRING uDevName;
        RtlInitUnicodeString(&uDevName, fullPath.c_str());

        NtDefs::OBJECT_ATTRIBUTES devAttr = {
            sizeof(NtDefs::OBJECT_ATTRIBUTES),
            NULL,
            &uDevName,
            0x00000040, // OBJ_CASE_INSENSITIVE
            NULL,
            NULL
        };

        NtDefs::IO_STATUS_BLOCK ioStatus;
        HANDLE hTemp = NULL;

        // 3. Открываем устройство
        NtDefs::NTSTATUS status = NtOpenFile(&hTemp, 0xC0100000, &devAttr, &ioStatus, 0, 0x00000020);

        if (status < 0) {
            //std::wcout << L"[ERROR] Failed to open device. Status: 0x" << std::hex << status << std::dec << std::endl;
            return INVALID_HANDLE_VALUE;
        }

        // 4. Пинг
        DriverRequest req = { 0 };
        status = NtDeviceIoControlFile(hTemp, NULL, NULL, NULL, &ioStatus, IO_PING, &req, sizeof(req), &req, sizeof(req));

        if (status >= 0) {
            if (req.Output == PING_MAGIC) {
                //std::wcout << L"[SUCCESS] PING OK! Magic matched. Driver connected." << std::endl;

                __delete_file(ecrypt(L"\\??\\C:\\log.txt"));
                // ---------------------------------

                return hTemp;
            }
            else {
                //std::wcout << L"[ERROR] PING failed. Magic mismatch. Got: " << std::hex << req.Output << std::endl;
            }
        }
        else {
            //std::wcout << L"[ERROR] IO_PING IOCTL failed. Status: 0x" << std::hex << status << std::dec << std::endl;
        }

        NtClose(hTemp);
        return INVALID_HANDLE_VALUE;
    }
public:
    uintptr_t client = 0;
    uintptr_t engine = 0;
    void* get_handle() const { return hDriver; }
public:
    __scrt_km() {
        __scrt_intf();
    }

    ~__scrt_km() {
        if (hDriver != INVALID_HANDLE_VALUE && NtClose) {
            NtClose(hDriver);
        }
    }

    bool Connect() {
        hDriver = __scrt_sfd();
        return hDriver != INVALID_HANDLE_VALUE;
    }

    bool enable_protection() {
        if (hDriver == INVALID_HANDLE_VALUE) return false;

        DriverRequest req = { 0 };

        // ВАЖНО: Передаем СВОЙ PID, а не targetPid
        req.ProcessId = GetCurrentProcessId();

        NtDefs::IO_STATUS_BLOCK io;
        NtDefs::NTSTATUS status = NtDeviceIoControlFile(hDriver, NULL, NULL, NULL, &io,
            IO_SET_PROTECTED_PID, &req, sizeof(req), &req, sizeof(req));

        return (status >= 0 && req.Output == 1);
    }

    bool __scrt_a(const std::wstring& processName) {
        if (!NtQuerySystemInformation) return false;

        ULONG bufferSize = 1024 * 1024;
        std::vector<BYTE> buffer(bufferSize);

        if (NtQuerySystemInformation(5, buffer.data(), bufferSize, &bufferSize) >= 0) {
            auto spi = (NtDefs::PSYSTEM_PROCESS_INFORMATION)buffer.data();
            while (true) {
                if (spi->ImageName.Buffer) {
                    if (_wcsicmp(spi->ImageName.Buffer, processName.c_str()) == 0) {
                        targetPid = (DWORD)(ULONG_PTR)spi->UniqueProcessId;
                        return true;
                    }
                }
                if (spi->NextEntryOffset == 0) break;
                spi = (NtDefs::PSYSTEM_PROCESS_INFORMATION)((PBYTE)spi + spi->NextEntryOffset);
            }
        }
        return false;
    }

    DWORD __scrt_gp() const { return targetPid; }

    uint64_t __scrt_gmb(const std::wstring& moduleName = L"") {
        if (hDriver == INVALID_HANDLE_VALUE || targetPid == 0) return 0;

        DriverRequest req = { 0 };
        req.ProcessId = targetPid;
        if (!moduleName.empty()) {
            wcscpy_s(req.ModuleName, moduleName.c_str());
        }

        NtDefs::IO_STATUS_BLOCK ioStatus;
        NtDeviceIoControlFile(hDriver, NULL, NULL, NULL, &ioStatus,
            IO_GET_CLIENT_BASE, &req, sizeof(req), &req, sizeof(req));

        return req.Output;
    }

    // --- ФУНКЦИИ ЧТЕНИЯ/ЗАПИСИ ---

    bool __scrt_rr(uint64_t address, void* buffer, size_t size) {
        if (hDriver == INVALID_HANDLE_VALUE || targetPid == 0) return false;
        DriverRequest req = { 0 };
        req.ProcessId = targetPid;
        req.TargetAddress = address;
        req.BufferAddress = (ULONGLONG)buffer;
        req.Size = size;

        NtDefs::IO_STATUS_BLOCK io;
        return NtDeviceIoControlFile(hDriver, NULL, NULL, NULL, &io,
            IO_READ_MEM, &req, sizeof(req), &req, sizeof(req)) >= 0;
    }

    template <typename T>
    T readvm(uint64_t address) {
        T buffer{};
        __scrt_rr(address, &buffer, sizeof(T));
        return buffer;
    }

    /*
        bool __scrt_wr(uint64_t address, void* buffer, size_t size) {
            if (hDriver == INVALID_HANDLE_VALUE || targetPid == 0) return false;
            DriverRequest req = { 0 };
            req.ProcessId = targetPid;
            req.TargetAddress = address;
            req.BufferAddress = (ULONGLONG)buffer;
            req.Size = size;

            NtDefs::IO_STATUS_BLOCK io;
            if (NtDeviceIoControlFile(hDriver, NULL, NULL, NULL, &io,
                IO_WRITE_MEM, &req, sizeof(req), &req, sizeof(req)) >= 0) {
                return req.Output == 1;
            }
            return false;
        }

        template <typename T>
        bool _std__operator___syswvm(uint64_t address, T value) {
            return __scrt_wr(address, &value, sizeof(T));
        }

            // --- ФУНКЦИИ ИНЖЕКТОРА ---

        uint64_t __scrt_alloc(size_t size, DWORD protect = PAGE_EXECUTE_READWRITE) {
            if (hDriver == INVALID_HANDLE_VALUE || targetPid == 0) return 0;
            DriverRequest req = { 0 };
            req.ProcessId = targetPid;
            req.Size = size;
            req.AllocationType = MEM_COMMIT | MEM_RESERVE;
            req.Protection = protect;

            NtDefs::IO_STATUS_BLOCK io;
            NtDeviceIoControlFile(hDriver, NULL, NULL, NULL, &io,
                IO_ALLOC_MEM, &req, sizeof(req), &req, sizeof(req));

            return req.Output;
        }

        DWORD __scrt_prtct(uint64_t address, size_t size, DWORD newProtect) {
            if (hDriver == INVALID_HANDLE_VALUE || targetPid == 0) return 0;
            DriverRequest req = { 0 };
            req.ProcessId = targetPid;
            req.TargetAddress = address;
            req.Size = size;
            req.Protection = newProtect;

            NtDefs::IO_STATUS_BLOCK io;
            NtDeviceIoControlFile(hDriver, NULL, NULL, NULL, &io,
                IO_PROTECT_MEM, &req, sizeof(req), &req, sizeof(req));

            return (DWORD)req.Output;
        }

        bool __scrt_fr(uint64_t address) {
            if (hDriver == INVALID_HANDLE_VALUE || targetPid == 0) return false;
            DriverRequest req = { 0 };
            req.ProcessId = targetPid;
            req.TargetAddress = address;

            NtDefs::IO_STATUS_BLOCK io;
            NtDeviceIoControlFile(hDriver, NULL, NULL, NULL, &io,
                IO_FREE_MEM, &req, sizeof(req), &req, sizeof(req));

            return req.Output == 1;
        }

        DWORD __scrt_gmt() {
            if (!NtQuerySystemInformation || targetPid == 0) return 0;

            ULONG bufferSize = 1024 * 1024; // 1MB
            std::vector<BYTE> buffer(bufferSize);

            NTSTATUS status = NtQuerySystemInformation(5, buffer.data(), bufferSize, &bufferSize);
            if (status < 0) return 0;

            auto spi = (NtDefs::PSYSTEM_PROCESS_INFORMATION)buffer.data();

            while (true) {
                if ((DWORD)(uintptr_t)spi->UniqueProcessId == targetPid) {
                    // Нашли наш процесс
                    if (spi->NumberOfThreads > 0) {
                        // Возвращаем первый поток.
                        // Обычно Threads[0] это Main Thread.
                        // Если он не сработает, можно попробовать перебрать Threads[i]
                        return (DWORD)(uintptr_t)spi->Threads[0].ClientId.UniqueThread;
                    }
                    return 0;
                }

                if (spi->NextEntryOffset == 0) break;
                spi = (NtDefs::PSYSTEM_PROCESS_INFORMATION)((PBYTE)spi + spi->NextEntryOffset);
            }
            return 0;
        }

        bool __scrt_ea(uint64_t address) {
            if (hDriver == INVALID_HANDLE_VALUE || targetPid == 0) return false;
            //exec apc
            // 1. Находим ID главного потока
            DWORD tid = __scrt_gmt();
            if (tid == 0) return false;

            //std::cout << "[*] Targeting Thread ID: " << tid << std::endl;

            // 2. Отправляем запрос драйверу
            DriverRequest req = { 0 };
            req.ProcessId = targetPid;
            req.ThreadId = tid;        // <--- ID потока
            req.TargetAddress = address; // <--- Адрес шеллкода

            NtDefs::IO_STATUS_BLOCK io;
            if (NtDeviceIoControlFile(hDriver, NULL, NULL, NULL, &io, IO_APC_INJECT, &req, sizeof(req), &req, sizeof(req)) >= 0) {
                return req.Output == 1;
            }
            return false;
        }
    */

    bool _send_em_ioctl(LONG dx, LONG dy, USHORT flags) {
        if (hDriver == INVALID_HANDLE_VALUE) return false;

        DriverRequest req = { 0 };
        req.Mouse.DeltaX = dx;
        req.Mouse.DeltaY = dy;
        req.Mouse.ButtonFlags = flags;

        NtDefs::IO_STATUS_BLOCK io;
        NtDefs::NTSTATUS status = NtDeviceIoControlFile(hDriver, NULL, NULL, NULL, &io,
            IO_MOUSE_EVENT, &req, sizeof(req), &req, sizeof(req));

        return NT_SUCCESS(status) && (req.Output == 1);
    }

    void mouse_click(int delay_between_click) {
        _send_em_ioctl(0, 0, MOUSE_LEFT_BUTTON_DOWN);
        if (delay_between_click > 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_between_click));
        _send_em_ioctl(0, 0, MOUSE_LEFT_BUTTON_UP);
    }

    bool mouse_move_(int delta_x, int delta_y) {
        return _send_em_ioctl(delta_x, delta_y, 0);
    }

    bool mouse_move__(int x, int y) {
        int jitter_x = static_cast<int>(x * (0.003f + (rand() % 10) * 0.001f));
        int jitter_y = static_cast<int>(y * (0.004f + (rand() % 10) * 0.001f));
        return _send_em_ioctl(x + jitter_x, y + jitter_y, 0);
        int random_delay = 1 + (rand() % 3); // 1-5 мс
        std::this_thread::sleep_for(std::chrono::milliseconds(random_delay));
    }

    // Simple non-blocking mouse move wrapper

    bool mouse_move(int x, int y) {
        if (x == 0 && y == 0) return true;
        int random_delay = 1 + (rand() % 3); // 1-5 мс
        std::this_thread::sleep_for(std::chrono::milliseconds(random_delay));
        return _send_em_ioctl(x, y, 0);
    }

    bool mouse_move_old(int x, int y) {
        if (x == 0 && y == 0) return true;

        int random_delay = 1 + (rand() % 3); // 1-5 мс

        // Вычисляем длину вектора движения
        float distance = std::sqrt(x * x + y * y);

        // Если движение маленькое (микро-коррекция), отправляем сразу (с минимальным джиттером)
        if (distance < 10.0f) {
            // Легкий шум для обхода паттерн-детекта (опционально)
            // int jitter_x = (rand() % 2) ? 0 : (rand() % 3 - 1); 
            // return _send_em_ioctl(x + jitter_x, y, 0);
            return _send_em_ioctl(x, y, 0);
        }

        // Если движение большое, разбиваем его на шаги (эмуляция 1000Hz мыши)
        // Чем больше дистанция, тем больше шагов
        int steps = (int)(distance / 10.0f);
        if (steps < 2) steps = 2;
        if (steps > 10) steps = 5; // 20 Ограничитель, чтобы не зависнуть надолго

        float x_step = (float)x / steps;
        float y_step = (float)y / steps;

        float accum_x = 0.0f;
        float accum_y = 0.0f;

        for (int i = 0; i < steps; i++) {
            accum_x += x_step;
            accum_y += y_step;

            int dx = (int)accum_x;
            int dy = (int)accum_y;

            if (dx != 0 || dy != 0) {
                _send_em_ioctl(dx, dy, 0);
                accum_x -= dx;
                accum_y -= dy;

                // КРИТИЧЕСКИ ВАЖНО: Задержка внутри цикла для имитации физического движения
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                //std::this_thread::sleep_for(std::chrono::milliseconds(random_delay));
            }
        }

        return true;
    }
};

inline __scrt_km g_driver;
