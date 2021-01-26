#pragma once
#include <ntifs.h>

inline auto dereference(uintptr_t address, unsigned int offset) -> uintptr_t
{
    if (address == 0)
        return 0;

    return address + (int)((*(int*)(address + offset) + offset) + sizeof(int));
}
inline auto relative(uintptr_t address, unsigned int size) -> PVOID
{
    if (address == 0)
        return 0;

    return ((PVOID)((unsigned char*)(address)+*(int*)((unsigned char*)(address)+((size)-(INT)sizeof(INT))) + (size)));
}
inline auto compare_data(const unsigned char* pData, const unsigned char* bMask, const char* szMask) -> bool
{
    for (; *szMask; ++szMask, ++pData, ++bMask)
        if (*szMask == 'x' && *pData != *bMask)
            return 0;

    return (*szMask) == 0;
}
inline auto find_pattern2(UINT64 dwAddress, UINT64 dwLen, unsigned char* bMask, const char* szMask) -> ULONGLONG
{
    for (ULONGLONG i = 0; i < dwLen; i++)
        if (compare_data((unsigned char*)(dwAddress + i), bMask, szMask))
            return (ULONGLONG)(dwAddress + i);

    return 0;
}
template <typename t = void*>
inline auto find_pattern(void* start, size_t length, const char* pattern, const char* mask) -> t
{
    const auto data = static_cast<const char*>(start);
    const auto pattern_length = strlen(mask);

    for (size_t i = 0; i <= length - pattern_length; i++)
    {
        bool accumulative_found = true;

        for (size_t j = 0; j < pattern_length; j++)
        {
            if (!MmIsAddressValid(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(data) + i + j)))
            {
                accumulative_found = false;
                break;
            }

            if (data[i + j] != pattern[j] && mask[j] != '?')
            {
                accumulative_found = false;
                break;
            }
        }

        if (accumulative_found)
        {
            return (t)(reinterpret_cast<uintptr_t>(data) + i);
        }
    }

    return (t)nullptr;
}
