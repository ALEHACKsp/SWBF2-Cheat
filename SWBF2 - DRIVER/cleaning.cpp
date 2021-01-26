#include <ntifs.h>
#include <ntstrsafe.h>
#include <ntimage.h>

#include "cleaning.h"
#include "ntos.h"
#include "utils.h"
#include "skCrypter.h"

using namespace driver;

uintptr_t get_kernel_address(const char* name, size_t& size)
{
	NTSTATUS status = STATUS_SUCCESS;
	ULONG neededSize = 0;

	ZwQuerySystemInformation(
		SystemModuleInformation,
		&neededSize,
		0,
		&neededSize
	);

	PSYSTEM_MODULE_INFORMATIONN pModuleList;

	pModuleList = (PSYSTEM_MODULE_INFORMATIONN)ExAllocatePool(NonPagedPool, neededSize);

	if (!pModuleList) {
		return 0;
	}

	status = ZwQuerySystemInformation(SystemModuleInformation,
		pModuleList,
		neededSize,
		0
	);

	ULONG i = 0;
	uintptr_t address = 0;

	for (i = 0; i < pModuleList->ModuleCount; i++)
	{
		SYSTEM_MODULEE mod = pModuleList->Modules[i];

		address = uintptr_t(pModuleList->Modules[i].Base);
		size = uintptr_t(pModuleList->Modules[i].Size);
		if (strstr(mod.ImageName, name) != NULL)
			break;
	}

	ExFreePool(pModuleList);

	return address;
}

PVOID resolve_relative_address(PVOID Instruction, ULONG OffsetOffset, ULONG InstructionSize)
{
	ULONG_PTR Instr = (ULONG_PTR)Instruction;
	LONG RipOffset = *(PLONG)(Instr + OffsetOffset);
	PVOID ResolvedAddr = (PVOID)(Instr + InstructionSize + RipOffset);

	return ResolvedAddr;
}

ULONGLONG get_exported_function(const ULONGLONG mod, const char* name)
{
	const auto dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(mod);
	const auto nt_headers = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<ULONGLONG>(dos_header) + dos_header->e_lfanew);

	const auto data_directory = nt_headers->OptionalHeader.DataDirectory[0];
	const auto export_directory = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(mod + data_directory.VirtualAddress);

	const auto address_of_names = reinterpret_cast<ULONG*>(mod + export_directory->AddressOfNames);

	for (size_t i = 0; i < export_directory->NumberOfNames; i++)
	{
		const auto function_name = reinterpret_cast<const char*>(mod + address_of_names[i]);

		if (!_stricmp(function_name, name))
		{
			const auto name_ordinal = reinterpret_cast<unsigned short*>(mod + export_directory->AddressOfNameOrdinals)[i];

			const auto function_rva = mod + reinterpret_cast<ULONG*>(mod + export_directory->AddressOfFunctions)[name_ordinal];
			return function_rva;
		}
	}

	return 0;
}

unsigned char random_number()
{
	auto ntoskrnl = skCrypt("ntoskrnl.exe");
	auto systemRoutine = skCrypt("MmGetSystemRoutineAddress");

	size_t size;
	auto mod = get_kernel_address(ntoskrnl.decrypt(), size);

	auto cMmGetSystemRoutineAddress = reinterpret_cast<decltype(&MmGetSystemRoutineAddress)>(get_exported_function((uintptr_t)mod, systemRoutine.decrypt()));

	UNICODE_STRING routineName = RTL_CONSTANT_STRING(L"RtlRandom");
	auto cRtlRandom = reinterpret_cast<decltype(&RtlRandom)>(cMmGetSystemRoutineAddress(&routineName));

	ULONG seed = 1234765;
	ULONG rand = cRtlRandom(&seed) % 100;

	unsigned char randint = 0;

	if (rand >= 101 || rand <= -1)
		randint = 72;

	ntoskrnl.clear();
	systemRoutine.clear();

	return (unsigned char)(rand);
}

PERESOURCE get_ps_loaded()
{
	auto ntoskrnl = skCrypt("ntoskrnl.exe");
	auto systemRoutine = skCrypt("MmGetSystemRoutineAddress");

	size_t size;
	auto mod = get_kernel_address(ntoskrnl.decrypt(), size);

	auto cMmGetSystemRoutineAddress = reinterpret_cast<decltype(&MmGetSystemRoutineAddress)>(get_exported_function((uintptr_t)mod, systemRoutine.decrypt()));

	ERESOURCE PsLoadedModuleResource;
	UNICODE_STRING routineName = RTL_CONSTANT_STRING(L"PsLoadedModuleResource");
	auto cPsLoadedModuleResource = reinterpret_cast<decltype(&PsLoadedModuleResource)>(cMmGetSystemRoutineAddress(&routineName));

	ntoskrnl.clear();
	systemRoutine.clear();

	return cPsLoadedModuleResource;
}

PRTL_AVL_TABLE get_piddb_table()
{
	auto ntoskrnl = skCrypt("ntoskrnl.exe");
	auto buildNumberHighSig = skCrypt("\x48\x8d\x0d\x00\x00\x00\x00\xe8\x00\x00\x00\x00\x3d\x00\x00\x00\x00\x0f\x83");
	auto buildNumberHighMask = skCrypt("xxx????x????x????xx");
	auto buildNumberLowSig = skCrypt("\x48\x8D\x0D\x00\x00\x00\x00\x4C\x89\x35\x00\x00\x00\x00\x49");
	auto buildNumberLowMask = skCrypt("xxx????xxx????x");

	size_t size;
	uintptr_t ntos_base = get_kernel_address(ntoskrnl.decrypt(), size);

	RTL_OSVERSIONINFOW osVersion = { 0 };
	osVersion.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);
	RtlGetVersion(&osVersion);

	PRTL_AVL_TABLE PiDDBCacheTable = nullptr;

	if (osVersion.dwBuildNumber >= 18362) {
		PiDDBCacheTable = (PRTL_AVL_TABLE)dereference(find_pattern<uintptr_t>((void*)ntos_base, size, buildNumberHighSig.decrypt(), buildNumberHighMask.decrypt()), 3);
	}
	else if (osVersion.dwBuildNumber >= 17134) {
		PiDDBCacheTable = (PRTL_AVL_TABLE)dereference(find_pattern<uintptr_t>((void*)ntos_base, size, buildNumberLowSig.decrypt(), buildNumberLowMask.decrypt()), 3);
	}

	if (!PiDDBCacheTable)
		return 0;

	ntoskrnl.clear();
	buildNumberHighSig.clear();
	buildNumberHighMask.clear();
	buildNumberLowSig.clear();
	buildNumberLowMask.clear();

	return PiDDBCacheTable;
}

PERESOURCE get_piddb_lock()
{
	auto ntoskrnl = skCrypt("ntoskrnl.exe");
	auto piddbLockSig = skCrypt("\x48\x8d\x0d\x00\x00\x00\x00\xe8\x00\x00\x00\x00\x48\x8b\x0d\x00\x00\x00\x00\x33\xdb");
	auto piddbLockMask = skCrypt("xxx????x????xxx????xx");

	size_t size;
	uintptr_t ntos_base = get_kernel_address(ntoskrnl.decrypt(), size);

	PERESOURCE PiDDBLock = (PERESOURCE)dereference(find_pattern<uintptr_t>((void*)ntos_base, size, piddbLockSig.decrypt(), piddbLockMask.decrypt()), 3);

	if (!PiDDBLock)
		return 0;

	ntoskrnl.clear();
	piddbLockSig.clear();
	piddbLockMask.clear();

	return PiDDBLock;
}

bool cleaning::verify_piddb()
{
	return (get_piddb_lock() != 0 && get_piddb_table() != 0);
}

bool cleaning::clean_piddb()
{
	PERESOURCE PiDDBLock = get_piddb_lock();
	PRTL_AVL_TABLE PiDDBCacheTable = get_piddb_table();

	PIDDBCACHE_ENTRY lookupEntry = { };

	lookupEntry.DriverName = cleaning::driver_name;
	lookupEntry.TimeDateStamp = cleaning::driver_timestamp;

	ExAcquireResourceExclusiveLite(PiDDBLock, TRUE);

	auto pFoundEntry = (PPIDDBCACHE_ENTRY)RtlLookupElementGenericTableAvl(PiDDBCacheTable, &lookupEntry);
	if (pFoundEntry == nullptr)
	{
		ExReleaseResourceLite(PiDDBLock);
		return false;
	}

	RemoveEntryList(&pFoundEntry->List);
	RtlDeleteElementGenericTableAvl(PiDDBCacheTable, pFoundEntry);

	ExReleaseResourceLite(PiDDBLock);

	return true;
}

bool is_unload_empty(PMM_UNLOADED_DRIVER entry)
{
	if (entry->Name.MaximumLength == 0 || entry->Name.Length == 0 || entry->Name.Buffer == NULL)
		return true;

	return false;
}

PMM_UNLOADED_DRIVER get_mmu_address()
{
	auto ntoskrnl = skCrypt("ntoskrnl.exe");
	auto mmuSig = skCrypt("\x4C\x8B\x15\x00\x00\x00\x00\x4C\x8B\xC9");
	auto mmuMask = skCrypt("xxx????xxx");

	size_t size;
	uintptr_t ntos_base = get_kernel_address(ntoskrnl.decrypt(), size);

	PVOID MmUnloadedDriversInstr = (PVOID)find_pattern2((UINT64)ntos_base, size,
		(unsigned char*)mmuSig.decrypt(), mmuMask.decrypt());

	if (MmUnloadedDriversInstr == NULL)
		return NULL;

	ntoskrnl.clear();
	mmuSig.clear();
	mmuMask.clear();

	return *(PMM_UNLOADED_DRIVER*)resolve_relative_address(MmUnloadedDriversInstr, 3, 7);
}

PULONG get_mml_address()
{
	auto ntoskrnl = skCrypt("ntoskrnl.exe");
	auto mmlSig = skCrypt("\x8B\x05\x00\x00\x00\x00\x83\xF8\x32");
	auto mmlMask = skCrypt("xx????xxx");

	size_t size;
	uintptr_t ntos_base = get_kernel_address(ntoskrnl.decrypt(), size);

	PVOID mmlastunloadeddriverinst = (PVOID)find_pattern2((UINT64)ntos_base, size,
		(unsigned char*)mmlSig.decrypt(), mmlMask.decrypt());

	if (mmlastunloadeddriverinst == NULL)
		return { };

	ntoskrnl.clear();
	mmlSig.clear();
	mmlMask.clear();

	return (PULONG)resolve_relative_address(mmlastunloadeddriverinst, 2, 6);

}

bool cleaning::verify_mmu()
{
	return (get_mmu_address() != NULL && get_mml_address() != NULL);
}

bool is_mmu_filled()
{
	for (ULONG idx = 0; idx < MM_UNLOADED_DRIVERS_SIZE; ++idx)
	{
		PMM_UNLOADED_DRIVER entry = &get_mmu_address()[idx];
		if (is_unload_empty(entry))
			return false;
	}
	return true;
}

bool cleaning::clean_mmu()
{
	auto ps_loaded = get_ps_loaded();

	ExAcquireResourceExclusiveLite(ps_loaded, TRUE);

	BOOLEAN Modified = false;
	BOOLEAN Filled = is_mmu_filled();

	UNICODE_STRING DriverName = cleaning::driver_name;

	for (ULONG Index = 0; Index < MM_UNLOADED_DRIVERS_SIZE; ++Index)
	{
		PMM_UNLOADED_DRIVER Entry = &get_mmu_address()[Index];

		if (cleaning::debug)
			DbgPrint("mmu driver # %i name %ws", Index, Entry->Name.Buffer);

		if (Modified)
		{
			PMM_UNLOADED_DRIVER PrevEntry = &get_mmu_address()[Index - 1];
			RtlCopyMemory(PrevEntry, Entry, sizeof(MM_UNLOADED_DRIVER));

			if (Index == MM_UNLOADED_DRIVERS_SIZE - 1)
			{
				RtlFillMemory(Entry, sizeof(MM_UNLOADED_DRIVER), 0);
			}
		}
		else if (RtlEqualUnicodeString(&DriverName, &Entry->Name, TRUE))
		{
			PVOID BufferPool = Entry->Name.Buffer;
			RtlFillMemory(Entry, sizeof(MM_UNLOADED_DRIVER), 0);
			ExFreePoolWithTag(BufferPool, 'TDmM');

			*get_mml_address() = (Filled ? MM_UNLOADED_DRIVERS_SIZE : *get_mml_address()) - 1;
			Modified = TRUE;
		}
	}

	if (Modified)
	{
		ULONG64 PreviousTime = 0;

		for (LONG Index = MM_UNLOADED_DRIVERS_SIZE - 2; Index >= 0; --Index)
		{
			PMM_UNLOADED_DRIVER Entry = &get_mmu_address()[Index];
			if (is_unload_empty(Entry))
			{
				continue;
			}

			if (PreviousTime != 0 && Entry->UnloadTime > PreviousTime)
			{
				Entry->UnloadTime = PreviousTime - random_number();
			}

			PreviousTime = Entry->UnloadTime;
		}

		clean_mmu();
	}

	ExReleaseResourceLite(ps_loaded);

	return Modified;

}

bool cleaning::clear_pool_tag(uint64_t address) 
{
	auto ntoskrnl = skCrypt("ntoskrnl.exe");
	size_t size;
	uintptr_t ntos_base = get_kernel_address(ntoskrnl.decrypt(), size);

	auto pageTableSizeSig = skCrypt("\x4C\x8B\x15\x00\x00\x00\x00\x48\x85");
	auto pageTableSizeMask = skCrypt("xxx????xx");

	auto pageTableSig = skCrypt("\x48\x8B\x15\x00\x00\x00\x00\x4C\x8D\x0D\x00\x00\x00\x00\x4C");
	auto pageTableMask = skCrypt("xxx????xxx????x");

	SIZE_T big_page_table_size = *(SIZE_T*)(find_pattern2(ntos_base, size, (unsigned char*)pageTableSizeSig.decrypt(), pageTableSizeSig.decrypt()), 3);
	PPOOL_TRACKER_BIG_PAGES big_page_table = *(PPOOL_TRACKER_BIG_PAGES*)(find_pattern2(ntos_base, size, (unsigned char*)pageTableSig.decrypt(), pageTableMask.decrypt()), 3);

	for (SIZE_T i = 0; i < big_page_table_size; i++) {
		if (!big_page_table[i].Key)
			continue;
		if (big_page_table[i].Key == address) {
			RtlZeroMemory(&big_page_table[i], sizeof(POOL_TRACKER_BIG_PAGES));
			DbgPrint("removed from pool");

		}

	}

	for (SIZE_T i = 0; i < big_page_table_size; i++) {
		if (!big_page_table[i].Key)
			continue;
		if (big_page_table[i].Key == address) {
			return false;
		}
	}

	ntoskrnl.clear();
	pageTableSizeSig.clear();
	pageTableSizeMask.clear();
	pageTableSig.clear();
	pageTableMask.clear();

	return true;

}

bool cleaning::clean_traces()
{
	bool status;

	if (cleaning::verify_mmu())
	{
		status = cleaning::clean_mmu();

		if (!status)
			DbgPrint("failed to clean mmu");
		else
			DbgPrint("cleaned mmu");
	}
	else
		DbgPrint("failed to verify mmu");

	if (cleaning::verify_piddb())
	{
		status = cleaning::clean_piddb();

		if (!status)
			DbgPrint("failed to clean piddb");

		else
			DbgPrint("cleaned piddb");
	}
	else
		DbgPrint("failed to verify piddb");

	return status;
}