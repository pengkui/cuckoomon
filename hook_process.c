/*
Cuckoo Sandbox - Automated Malware Analysis
Copyright (C) 2010-2012 Cuckoo Sandbox Developers

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <windows.h>
#include "hooking.h"
#include "ntapi.h"
#include "log.h"
#include "pipe.h"

static IS_SUCCESS_NTSTATUS();
static const char *module_name = "process";

typedef struct _PROCESS_BASIC_INFORMATION {
    PVOID Reserved1;
    void *PebBaseAddress;
    PVOID Reserved2[2];
    ULONG_PTR UniqueProcessId;
    PVOID Reserved3;
} PROCESS_BASIC_INFORMATION;

static DWORD GetPidFromHandle(HANDLE hProcess)
{
    PROCESS_BASIC_INFORMATION pbi = {}; ULONG ulSize;
    LONG (WINAPI *NtQueryInformationProcess)(HANDLE ProcessHandle,
        ULONG ProcessInformationClass, PVOID ProcessInformation,
        ULONG ProcessInformationLength, PULONG ReturnLength);

    *(FARPROC *) &NtQueryInformationProcess = GetProcAddress(
        LoadLibrary("ntdll"), "NtQueryInformationProcess");

    if(NtQueryInformationProcess != NULL && NtQueryInformationProcess(
            hProcess, 0, &pbi, sizeof(pbi), &ulSize) >= 0 &&
            ulSize == sizeof(pbi)) {
        return pbi.UniqueProcessId;
    }
    return 0;
}

HOOKDEF(NTSTATUS, WINAPI, NtCreateProcess,
    __out       PHANDLE ProcessHandle,
    __in        ACCESS_MASK DesiredAccess,
    __in_opt    POBJECT_ATTRIBUTES ObjectAttributes,
    __in        HANDLE ParentProcess,
    __in        BOOLEAN InheritObjectTable,
    __in_opt    HANDLE SectionHandle,
    __in_opt    HANDLE DebugPort,
    __in_opt    HANDLE ExceptionPort
) {
    NTSTATUS ret = Old_NtCreateProcess(ProcessHandle, DesiredAccess,
        ObjectAttributes, ParentProcess, InheritObjectTable, SectionHandle,
        DebugPort, ExceptionPort);
    LOQ("PO", "ProcessHandle", ProcessHandle, "FileName", ObjectAttributes);
    if(NT_SUCCESS(ret)) {
        pipe_write("PID:%d", GetPidFromHandle(*ProcessHandle));
    }
    return ret;
}

HOOKDEF(NTSTATUS, WINAPI, NtCreateProcessEx,
    __out       PHANDLE ProcessHandle,
    __in        ACCESS_MASK DesiredAccess,
    __in_opt    POBJECT_ATTRIBUTES ObjectAttributes,
    __in        HANDLE ParentProcess,
    __in        ULONG Flags,
    __in_opt    HANDLE SectionHandle,
    __in_opt    HANDLE DebugPort,
    __in_opt    HANDLE ExceptionPort,
    __in        BOOLEAN InJob
) {
    NTSTATUS ret = Old_NtCreateProcessEx(ProcessHandle, DesiredAccess,
        ObjectAttributes, ParentProcess, Flags, SectionHandle, DebugPort,
        ExceptionPort, InJob);
    LOQ("PO", "ProcessHandle", ProcessHandle, "FileName", ObjectAttributes);
    if(NT_SUCCESS(ret)) {
        pipe_write("PID:%d", GetPidFromHandle(*ProcessHandle));
    }
    return ret;
}

HOOKDEF(BOOL, WINAPI, CreateProcessInternalW,
    __in_opt    LPVOID lpUnknown1,
    __in_opt    LPWSTR lpApplicationName,
    __inout_opt LPWSTR lpCommandLine,
    __in_opt    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    __in_opt    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    __in        BOOL bInheritHandles,
    __in        DWORD dwCreationFlags,
    __in_opt    LPVOID lpEnvironment,
    __in_opt    LPWSTR lpCurrentDirectory,
    __in        LPSTARTUPINFO lpStartupInfo,
    __out       LPPROCESS_INFORMATION lpProcessInformation,
    __in_opt    LPVOID lpUnknown2
) {
    IS_SUCCESS_BOOL();

    BOOL ret = Old_CreateProcessInternalW(lpUnknown1, lpApplicationName,
        lpCommandLine, lpProcessAttributes, lpThreadAttributes,
        bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory,
        lpStartupInfo, lpProcessInformation, lpUnknown2);
    LOQ("uu3l2p", "ApplicationName", lpApplicationName,
        "CommandLine", lpCommandLine, "CreationFlags", dwCreationFlags,
        "ProcessId", lpProcessInformation->dwProcessId,
        "ThreadId", lpProcessInformation->dwThreadId,
        "ProcessHandle", lpProcessInformation->hProcess,
        "ThreadHandle", lpProcessInformation->hThread);
    if(ret != FALSE) {
        pipe_write("PID:%d", lpProcessInformation->dwProcessId);
    }
    return ret;
}

HOOKDEF(HANDLE, WINAPI, OpenProcess,
  __in  DWORD dwDesiredAccess,
  __in  BOOL bInheritHandle,
  __in  DWORD dwProcessId
) {
    IS_SUCCESS_HANDLE();

    HANDLE ret = Old_OpenProcess(dwDesiredAccess, bInheritHandle,
        dwProcessId);
    LOQ("ll", "DesiredAccess", dwDesiredAccess, "ProcessId", dwProcessId);
    return ret;
}

HOOKDEF(BOOL, WINAPI, TerminateProcess,
  __in  HANDLE hProcess,
  __in  UINT uExitCode
) {
    IS_SUCCESS_BOOL();

    BOOL ret = Old_TerminateProcess(hProcess, uExitCode);
    LOQ("pl", "ProcessHandle", hProcess, "ExitCode", uExitCode);
    return ret;
}

HOOKDEF(VOID, WINAPI, ExitProcess,
  __in  UINT uExitCode
) {
    IS_SUCCESS_VOID();

    int ret = 0;
    LOQ("l", "ExitCode", uExitCode);
    Old_ExitProcess(uExitCode);
}

HOOKDEF(BOOL, WINAPI, ShellExecuteExW,
  __inout  SHELLEXECUTEINFOW *pExecInfo
) {
    IS_SUCCESS_BOOL();

    BOOL ret = Old_ShellExecuteExW(pExecInfo);
    LOQ("2ul", pExecInfo->lpFile, pExecInfo->lpParameters, pExecInfo->nShow);
    return ret;
}

HOOKDEF(BOOL, WINAPI, ReadProcessMemory,
    __in   HANDLE hProcess,
    __in   LPCVOID lpBaseAddress,
    __out  LPVOID lpBuffer,
    __in   SIZE_T nSize,
    __out  SIZE_T *lpNumberOfBytesRead
) {
    IS_SUCCESS_BOOL();

    BOOL ret = Old_ReadProcessMemory(hProcess, lpBaseAddress, lpBuffer,
        nSize, lpNumberOfBytesRead);
    LOQ("2pB", "ProcessHandle", hProcess, "BaseAddress", lpBaseAddress,
        "Buffer", lpNumberOfBytesRead, lpBuffer);
    return ret;
}

HOOKDEF(BOOL, WINAPI, WriteProcessMemory,
    __in   HANDLE hProcess,
    __in   LPVOID lpBaseAddress,
    __in   LPCVOID lpBuffer,
    __in   SIZE_T nSize,
    __out  SIZE_T *lpNumberOfBytesWritten
) {
    IS_SUCCESS_BOOL();

    BOOL ret = Old_WriteProcessMemory(hProcess, lpBaseAddress, lpBuffer,
        nSize, lpNumberOfBytesWritten);
    LOQ("2pB", "ProcessHandle", hProcess, "BaseAddress", lpBaseAddress,
        "Buffer", lpNumberOfBytesWritten, lpBuffer);
    return ret;
}

HOOKDEF(LPVOID, WINAPI, VirtualAllocEx,
    __in      HANDLE hProcess,
    __in_opt  LPVOID lpAddress,
    __in      SIZE_T dwSize,
    __in      DWORD flAllocationType,
    __in      DWORD flProtect
) {
    IS_SUCCESS_HANDLE();

    LPVOID ret = Old_VirtualAllocEx(hProcess, lpAddress, dwSize,
        flAllocationType, flProtect);
    LOQ("pplll", "ProcessHandle", hProcess, "Address", lpAddress,
        "Size", dwSize, "AllocationType", flAllocationType,
        "Protection", flProtect);
    return ret;
}

HOOKDEF(BOOL, WINAPI, VirtualProtectEx,
    __in   HANDLE hProcess,
    __in   LPVOID lpAddress,
    __in   SIZE_T dwSize,
    __in   DWORD flNewProtect,
    __out  PDWORD lpflOldProtect
) {
    IS_SUCCESS_BOOL();

    BOOL ret = Old_VirtualProtectEx(hProcess, lpAddress, dwSize, flNewProtect,
        lpflOldProtect);
    LOQ("2p2l", "ProcessHandle", hProcess, "Address", lpAddress,
        "Size", dwSize, "Protection", flNewProtect);
    return ret;
}

HOOKDEF(BOOL, WINAPI, VirtualFreeEx,
    __in  HANDLE hProcess,
    __in  LPVOID lpAddress,
    __in  SIZE_T dwSize,
    __in  DWORD dwFreeType
) {
    IS_SUCCESS_BOOL();

    BOOL ret = Old_VirtualFreeEx(hProcess, lpAddress, dwSize, dwFreeType);
    LOQ("ppll", "ProcessHandle", hProcess, "Address", lpAddress,
        "Size", dwSize, "FreeType", dwFreeType);
    return ret;
}
