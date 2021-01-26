#pragma once
#include "ntos.h"

PEPROCESS pProcess;
PEPROCESS pProcess32;

UINT64 processBaseAddress;

PPEB pPeb64;
PPEB32 pPeb32;

HANDLE processID;

ULONG clientBaseAddress;
ULONG engineBaseAddress;

PVOID getSystemModuleBase(const char* moduleName);
PVOID getSystemModuleExport(const char* moduleName, LPCSTR routineName);
bool writeMemory(void* address, void* buffer, size_t size);
bool writeToReadOnlyMemory(void* address, void* buffer, size_t);
bool RVPM(PVOID address, PVOID buffer, SIZE_T size);
bool WVPM(PVOID address, PVOID buffer, SIZE_T size);
bool WVPM2(PVOID address, PVOID buffer, SIZE_T size);
UINT64 findProcess();
UINT64 findClientBaseAddress();
UINT64 findClientBaseAddressX86();


