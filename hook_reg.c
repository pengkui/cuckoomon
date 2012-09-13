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

static IS_SUCCESS_LONGREG();
static const char *module_name = "registry";

HOOKDEF(NTSTATUS, WINAPI, NtCreateKey,
    __out       PHANDLE KeyHandle,
    __in        ACCESS_MASK DesiredAccess,
    __in        POBJECT_ATTRIBUTES ObjectAttributes,
    __reserved  ULONG TitleIndex,
    __in_opt    PUNICODE_STRING Class,
    __in        ULONG CreateOptions,
    __out_opt   PULONG Disposition
) {
    NTSTATUS ret = Old_NtCreateKey(KeyHandle, DesiredAccess, ObjectAttributes,
        TitleIndex, Class, CreateOptions, Disposition);
    LOQ("PlOo", "KeyHandle", KeyHandle, "DesiredAccess", DesiredAccess,
        "ObjectAttributes", ObjectAttributes, "Class", Class);
    return ret;
}

HOOKDEF(NTSTATUS, WINAPI, NtOpenKey,
    __out  PHANDLE KeyHandle,
    __in   ACCESS_MASK DesiredAccess,
    __in   POBJECT_ATTRIBUTES ObjectAttributes
) {
    NTSTATUS ret = Old_NtOpenKey(KeyHandle, DesiredAccess, ObjectAttributes);
    LOQ("PlO", "KeyHandle", KeyHandle, "DesiredAccess", DesiredAccess,
        "ObjectAttributes", ObjectAttributes);
    return ret;
}

HOOKDEF(NTSTATUS, WINAPI, NtOpenKeyEx,
    __out  PHANDLE KeyHandle,
    __in   ACCESS_MASK DesiredAccess,
    __in   POBJECT_ATTRIBUTES ObjectAttributes,
    __in   ULONG OpenOptions
) {
    NTSTATUS ret = Old_NtOpenKeyEx(KeyHandle, DesiredAccess, ObjectAttributes,
        OpenOptions);
    LOQ("PlO", "KeyHandle", KeyHandle, "DesiredAccess", DesiredAccess,
        "ObjectAttributes", ObjectAttributes);
    return ret;
}

HOOKDEF(NTSTATUS, WINAPI, NtRenameKey,
    __in  HANDLE KeyHandle,
    __in  PUNICODE_STRING NewName
) {
    NTSTATUS ret = Old_NtRenameKey(KeyHandle, NewName);
    LOQ("po", "KeyHandle", KeyHandle, "NewName", NewName);
    return ret;
}

HOOKDEF(NTSTATUS, WINAPI, NtReplaceKey,
    __in  POBJECT_ATTRIBUTES NewHiveFileName,
    __in  HANDLE KeyHandle,
    __in  POBJECT_ATTRIBUTES BackupHiveFileName
) {
    NTSTATUS ret = Old_NtReplaceKey(NewHiveFileName, KeyHandle,
        BackupHiveFileName);
    LOQ("pOO", "KeyHandle", KeyHandle, "NewHiveFileName", NewHiveFileName,
        "BackupHiveFileName", BackupHiveFileName);
    return ret;
}

HOOKDEF(NTSTATUS, WINAPI, NtEnumerateKey,
    __in       HANDLE KeyHandle,
    __in       ULONG Index,
    __in       KEY_INFORMATION_CLASS KeyInformationClass,
    __out_opt  PVOID KeyInformation,
    __in       ULONG Length,
    __out      PULONG ResultLength
) {
    NTSTATUS ret = Old_NtEnumerateKey(KeyHandle, Index, KeyInformationClass,
        KeyInformation, Length, ResultLength);
    LOQ("pl", "KeyHandle", KeyHandle, "Index", Index);
    return ret;
}

HOOKDEF(NTSTATUS, WINAPI, NtEnumerateValueKey,
    __in       HANDLE KeyHandle,
    __in       ULONG Index,
    __in       KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    __out_opt  PVOID KeyValueInformation,
    __in       ULONG Length,
    __out      PULONG ResultLength
) {
    NTSTATUS ret = Old_NtEnumerateValueKey(KeyHandle, Index,
        KeyValueInformationClass, KeyValueInformation, Length, ResultLength);
    LOQ("pll", "KeyHandle", KeyHandle, "Index", Index,
        "KeyValueInformationClass", KeyValueInformationClass);
    return ret;
}

HOOKDEF(NTSTATUS, WINAPI, NtSetValueKey,
    __in      HANDLE KeyHandle,
    __in      PUNICODE_STRING ValueName,
    __in_opt  ULONG TitleIndex,
    __in      ULONG Type,
    __in_opt  PVOID Data,
    __in      ULONG DataSize
) {
    NTSTATUS ret = Old_NtSetValueKey(KeyHandle, ValueName, TitleIndex,
        Type, Data, DataSize);
    LOQ("polb", "KeyHandle", KeyHandle, "ValueName", ValueName, "Type", Type,
        "Buffer", DataSize, Data);
    return ret;
}

HOOKDEF(NTSTATUS, WINAPI, NtQueryValueKey,
    __in       HANDLE KeyHandle,
    __in       PUNICODE_STRING ValueName,
    __in       KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    __out_opt  PVOID KeyValueInformation,
    __in       ULONG Length,
    __out      PULONG ResultLength
) {
    NTSTATUS ret = Old_NtQueryValueKey(KeyHandle, ValueName,
        KeyValueInformationClass, KeyValueInformation, Length, ResultLength);
    LOQ("poSl", "KeyHandle", KeyHandle, "ValueName", ValueName,
        "KeyValueInformation", Length, KeyValueInformation,
        "KeyValueInformationClass", KeyValueInformationClass);
    return ret;
}

HOOKDEF(NTSTATUS, WINAPI, NtQueryMultipleValueKey,
    __in       HANDLE KeyHandle,
    __inout    PKEY_VALUE_ENTRY ValueEntries,
    __in       ULONG EntryCount,
    __out      PVOID ValueBuffer,
    __inout    PULONG BufferLength,
    __out_opt  PULONG RequiredBufferLength
) {
    NTSTATUS ret = Old_NtQueryMultipleValueKey(KeyHandle, ValueEntries,
        EntryCount, ValueBuffer, BufferLength, RequiredBufferLength);
    LOQ("poB", "KeyHandle", KeyHandle, "ValueName", ValueEntries->ValueName,
        "ValueBuffer", BufferLength, ValueBuffer);
    return ret;
}

HOOKDEF(NTSTATUS, WINAPI, NtDeleteKey,
    __in  HANDLE KeyHandle
) {
    NTSTATUS ret = Old_NtDeleteKey(KeyHandle);
    LOQ("p", "KeyHandle", KeyHandle);
    return ret;
}

HOOKDEF(NTSTATUS, WINAPI, NtDeleteValueKey,
    __in  HANDLE KeyHandle,
    __in  PUNICODE_STRING ValueName
) {
    NTSTATUS ret = Old_NtDeleteValueKey(KeyHandle, ValueName);
    LOQ("po", "KeyHandle", KeyHandle, "ValueName", ValueName);
    return ret;
}

HOOKDEF(NTSTATUS, WINAPI, NtLoadKey,
    __in  POBJECT_ATTRIBUTES TargetKey,
    __in  POBJECT_ATTRIBUTES SourceFile
) {
    NTSTATUS ret = Old_NtLoadKey(TargetKey, SourceFile);
    LOQ("OO", "TargetKey", TargetKey, "SourceFile", SourceFile);
    return ret;
}

HOOKDEF(NTSTATUS, WINAPI, NtLoadKey2,
    __in  POBJECT_ATTRIBUTES TargetKey,
    __in  POBJECT_ATTRIBUTES SourceFile,
    __in  ULONG Flags
) {
    NTSTATUS ret = Old_NtLoadKey2(TargetKey, SourceFile, Flags);
    LOQ("OOl", "TargetKey", TargetKey, "SourceFile", SourceFile,
        "Flags", Flags);
    return ret;
}

HOOKDEF(NTSTATUS, WINAPI, NtLoadKeyEx,
    __in      POBJECT_ATTRIBUTES TargetKey,
    __in      POBJECT_ATTRIBUTES SourceFile,
    __in      ULONG Flags,
    __in_opt  HANDLE TrustClassKey
) {
    NTSTATUS ret = Old_NtLoadKeyEx(TargetKey, SourceFile, Flags,
        TrustClassKey);
    LOQ("pOOl", "TrustClassKey", TrustClassKey, "TargetKey", TargetKey,
        "SourceFile", SourceFile, "Flags", Flags);
    return ret;
}

HOOKDEF(NTSTATUS, WINAPI, NtQueryKey,
    __in       HANDLE KeyHandle,
    __in       KEY_INFORMATION_CLASS KeyInformationClass,
    __out_opt  PVOID KeyInformation,
    __in       ULONG Length,
    __out      PULONG ResultLength
) {
    NTSTATUS ret = Old_NtQueryKey(KeyHandle, KeyInformationClass,
        KeyInformation, Length, ResultLength);
    LOQ("pSl", "KeyHandle", KeyHandle,
        "KeyInformation", Length, KeyInformation,
        "KeyInformationClass", KeyInformationClass);
    return ret;
}

HOOKDEF(NTSTATUS, WINAPI, NtSaveKey,
    __in  HANDLE KeyHandle,
    __in  HANDLE FileHandle
) {
    NTSTATUS ret = Old_NtSaveKey(KeyHandle, FileHandle);
    LOQ("pp", "KeyHandle", KeyHandle, "FileHandle", FileHandle);
    return ret;
}

HOOKDEF(NTSTATUS, WINAPI, NtSaveKeyEx,
    __in  HANDLE KeyHandle,
    __in  HANDLE FileHandle,
    __in  ULONG Format
) {
    NTSTATUS ret = Old_NtSaveKeyEx(KeyHandle, FileHandle, Format);
    LOQ("ppl", "KeyHandle", KeyHandle, "FileHandle", FileHandle,
        "Format", Format);
    return ret;
}

HOOKDEF(LONG, WINAPI, RegQueryInfoKeyA,
    _In_         HKEY hKey,
    _Out_opt_    LPTSTR lpClass,
    _Inout_opt_  LPDWORD lpcClass,
    _Reserved_   LPDWORD lpReserved,
    _Out_opt_    LPDWORD lpcSubKeys,
    _Out_opt_    LPDWORD lpcMaxSubKeyLen,
    _Out_opt_    LPDWORD lpcMaxClassLen,
    _Out_opt_    LPDWORD lpcValues,
    _Out_opt_    LPDWORD lpcMaxValueNameLen,
    _Out_opt_    LPDWORD lpcMaxValueLen,
    _Out_opt_    LPDWORD lpcbSecurityDescriptor,
    _Out_opt_    PFILETIME lpftLastWriteTime
) {
    IS_SUCCESS_LONGREG();

    LONG ret = Old_RegQueryInfoKeyA(hKey, lpClass, lpcClass, lpReserved,
        lpcSubKeys, lpcMaxSubKeyLen, lpcMaxClassLen, lpcValues,
        lpcMaxValueNameLen, lpcMaxValueLen, lpcbSecurityDescriptor,
        lpftLastWriteTime);
    LOQ("pS6L", "KeyHandle", hKey, "Class", lpcClass ? *lpcClass : 0, lpClass,
        "SubKeyCount", lpcSubKeys, "MaxSubKeyLength", lpcMaxSubKeyLen,
        "MaxClassLength", lpcMaxClassLen, "ValueCount", lpcValues,
        "MaxValueNameLength", lpcMaxValueNameLen,
        "MaxValueLength", lpcMaxValueLen);
    return ret;
}

HOOKDEF(LONG, WINAPI, RegQueryInfoKeyW,
    _In_         HKEY hKey,
    _Out_opt_    LPWSTR lpClass,
    _Inout_opt_  LPDWORD lpcClass,
    _Reserved_   LPDWORD lpReserved,
    _Out_opt_    LPDWORD lpcSubKeys,
    _Out_opt_    LPDWORD lpcMaxSubKeyLen,
    _Out_opt_    LPDWORD lpcMaxClassLen,
    _Out_opt_    LPDWORD lpcValues,
    _Out_opt_    LPDWORD lpcMaxValueNameLen,
    _Out_opt_    LPDWORD lpcMaxValueLen,
    _Out_opt_    LPDWORD lpcbSecurityDescriptor,
    _Out_opt_    PFILETIME lpftLastWriteTime
) {
    IS_SUCCESS_LONGREG();

    LONG ret = Old_RegQueryInfoKeyW(hKey, lpClass, lpcClass, lpReserved,
        lpcSubKeys, lpcMaxSubKeyLen, lpcMaxClassLen, lpcValues,
        lpcMaxValueNameLen, lpcMaxValueLen, lpcbSecurityDescriptor,
        lpftLastWriteTime);
    LOQ("pU6L", "KeyHandle", hKey, "Class", lpcClass ? *lpcClass : 0, lpClass,
        "SubKeyCount", lpcSubKeys, "MaxSubKeyLength", lpcMaxSubKeyLen,
        "MaxClassLength", lpcMaxClassLen, "ValueCount", lpcValues,
        "MaxValueNameLength", lpcMaxValueNameLen,
        "MaxValueLength", lpcMaxValueLen);
    return ret;
}
