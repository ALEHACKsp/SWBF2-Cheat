#include "mem.h"

PVOID getSystemModuleBase(const char* moduleName)
{
	ULONG bytes = 0;
	NTSTATUS status = ZwQuerySystemInformation(SystemModuleInformation, NULL, bytes, &bytes);

	if (!bytes)
		return NULL;

	PRTL_PROCESS_MODULES modules = (PRTL_PROCESS_MODULES)ExAllocatePoolWithTag(NonPagedPool, bytes, 0x4E554C4C);

	status = ZwQuerySystemInformation(SystemModuleInformation, modules, bytes, &bytes);

	if (!NT_SUCCESS(status))
		return NULL;

	PRTL_PROCESS_MODULE_INFORMATION module = modules->Modules;
	PVOID moduleBase = 0, moduleSize = 0;

	for (ULONG i = 0; i < modules->NumberOfModules; i++)
	{
		if (strcmp((char*)module[i].FullPathName, moduleName) == 0)
		{
			moduleBase = module[i].ImageBase;
			moduleSize = (PVOID)module[i].ImageSize;
			break;

		}

	}

	if (modules)
		ExFreePoolWithTag(modules, NULL);

	if (moduleBase <= NULL)
		return NULL;

	return moduleBase;

}

PVOID getSystemModuleExport(const char* moduleName, LPCSTR routineName)
{
	PVOID lpModule = getSystemModuleBase(moduleName);

	if (!lpModule)
		return NULL;

	return RtlFindExportedRoutineByName(lpModule, routineName);

}

bool writeMemory(void* address, void* buffer, size_t size)
{
	if (!RtlCopyMemory(address, buffer, size))
		return false;
	else
		return true;

}

//essentially detour hook function with added security
bool writeToReadOnlyMemory(void* address, void* buffer, size_t size)
{
	//memory descriptor list
	//allocate memory descriptor list large enough to hold buffer
	PMDL Mdl = IoAllocateMdl(address, size, FALSE, FALSE, NULL);

	//check if Mdl is valid
	if (!Mdl)
		return false;

	//checks Mdl aquired pages to ensure validity, as well prevents loss of pages whilst using driver
	MmProbeAndLockPages(Mdl, KernelMode, IoReadAccess);
	//maps pages to a virtual address 
	PVOID mapping = MmMapLockedPagesSpecifyCache(Mdl, KernelMode, MmNonCached, NULL, FALSE, NormalPagePriority);
	//changes protections of aquired Mdl pages 
	MmProtectMdlSystemAddress(Mdl, PAGE_READWRITE);

	writeMemory(mapping, buffer, size);

	//unmaps locked pages
	MmUnmapLockedPages(mapping, Mdl);
	//frees pages
	MmUnlockPages(Mdl);
	//frees Mdl
	IoFreeMdl(Mdl);

	return true;

}

bool RVPM(PVOID address, PVOID buffer, SIZE_T size)
{
	SIZE_T bytes = 0;
	NTSTATUS status = STATUS_SUCCESS;

	status = MmCopyVirtualMemory(pProcess, address, (PEPROCESS)PsGetCurrentProcess(), buffer, size, KernelMode, &bytes);

	if (!NT_SUCCESS(status))
	{
		DbgPrint("[READ] Memory copy failed! \n");
		DbgPrint("[READ] Transferred bytes: %zu \n", bytes);
		return false;

	}
	else
	{
		DbgPrint("[READ] Memory copy succeeded! \n");
		DbgPrint("[READ] Transferred bytes: %zu \n", bytes);
		return true;

	}

}

bool WVPM(PVOID address, PVOID buffer, SIZE_T size)
{
	SIZE_T bytes = 0;
	NTSTATUS status = STATUS_SUCCESS;

	status = MmCopyVirtualMemory(PsGetCurrentProcess(), &buffer, pProcess, address, size, KernelMode, &bytes);

	if (!NT_SUCCESS(status))
	{
		DbgPrint("[WRITE] Memory copy failed! \n");
		DbgPrint("[WRITE] Transferred bytes: %zu \n", bytes);
		return false;

	}
	else
	{
		DbgPrint("[WRITE] Memory copy succeeded! \n");
		DbgPrint("[WRITE] Transferred bytes: %zu \n", bytes);
		return true;

	}

}

bool WVPM2(PVOID address, PVOID buffer, SIZE_T size)
{
	SIZE_T bytes = 0;
	NTSTATUS status = STATUS_SUCCESS;

	status = MmCopyVirtualMemory(PsGetCurrentProcess(), buffer, pProcess, address, size, KernelMode, &bytes);

	if (!NT_SUCCESS(status))
	{
		DbgPrint("[WRITE] Memory copy failed! \n");
		DbgPrint("[WRITE] Transferred bytes: %zu \n", bytes);
		return false;

	}
	else
	{
		DbgPrint("[WRITE] Memory copy succeeded! \n");
		DbgPrint("[WRITE] Transferred bytes: %zu \n", bytes);
		return true;

	}

}

//enumerate process 
UINT64 findProcess()
{
	ULONG bufferSize = 0;
	PSYSTEM_PROCESS_INFORMATION processInfo = { 0 };
	PVOID memory;
	NTSTATUS status = { 0 };
	char string[100];
	UNICODE_STRING processName;
	int compare = 0;

	do
	{
		if (ZwQuerySystemInformation(SystemProcessInformation, NULL, 0, &bufferSize) == STATUS_INFO_LENGTH_MISMATCH)
			if (bufferSize)
			{
				memory = ExAllocatePoolWithTag(PagedPool, (SIZE_T)bufferSize, (ULONG)"enoN");

				if (memory)
				{
					NTSTATUS status = ZwQuerySystemInformation(SystemProcessInformation, memory, bufferSize, &bufferSize);

					if (NT_SUCCESS(status))
					{
						processInfo = (PSYSTEM_PROCESS_INFORMATION)memory;

					}

					do
					{

						compare = 1;

						if (processInfo->ImageName.Length)
						{
							status = RtlStringCbPrintfA(string, _countof(string), "%ws \n", processInfo->ImageName.Buffer);

							if (NT_SUCCESS(status))
							{
								size_t length;
								status = RtlStringCbLengthA(string, _countof(string), &length);

								if (NT_SUCCESS(status))
								{
									RtlInitUnicodeString(&processName, L"starwarsbattlefrontii.exe");
									if (RtlCompareUnicodeString(&processInfo->ImageName, &processName, TRUE) == 0)
									{
										DbgPrint(string);
										DbgPrint("%lu \n", (UINT64)processInfo->UniqueProcessId);
										processID = processInfo->UniqueProcessId;

									}
								}

							}

						}
						processInfo = (PSYSTEM_PROCESS_INFORMATION)(processInfo->NextEntryOffset + (BYTE*)processInfo);

					} while (processInfo->NextEntryOffset);

				}
				ExFreePoolWithTag(memory, (ULONG)"enoN");

			}
	} while (compare == 0);

	return status;

}

UINT64 findClientBaseAddress()
{
	int platform64 = 64;

	if (NT_SUCCESS(PsLookupProcessByProcessId(processID, &pProcess)))
	{
		if (platform64 = 64)
		{
			pPeb64 = PsGetProcessPeb(pProcess);

			KAPC_STATE state64;
			KeStackAttachProcess(pProcess, &state64);

			processBaseAddress = (UINT64)PsGetProcessSectionBaseAddress(pProcess);
			DbgPrint("Process Base Address: %#X \n", processBaseAddress);

			for (PLIST_ENTRY pListEntry64 = pPeb64->Ldr->InMemoryOrderModuleList.Flink; pListEntry64 != &pPeb64->Ldr->InMemoryOrderModuleList; pListEntry64 = pListEntry64->Flink)
			{
				PLDR_DATA_TABLE_ENTRY pTableEntry64 = CONTAINING_RECORD(pListEntry64, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);

				DbgPrint("1. Base DLL Name: %wZ \n", pTableEntry64->BaseDllName);
				DbgPrint("2. DLL Base Address %p \n", pTableEntry64->DllBase);
				DbgPrint("3. DLL Entry Point %p \n", pTableEntry64->EntryPoint);

			}

			KeUnstackDetachProcess(&state64);

		}

	}
	return 0;

}

UINT64 findClientBaseAddressX86()
{
	int platform32 = 32;

	if (NT_SUCCESS(PsLookupProcessByProcessId(processID, &pProcess32)))
	{
		if (platform32 = 32)
		{
			pPeb32 = (PPEB32)PsGetProcessWow64Process(pProcess32);

			KAPC_STATE state32;
			KeStackAttachProcess(pProcess32, &state32);

			for (PLIST_ENTRY32 pListEntry32 = (PLIST_ENTRY32)((PPEB_LDR_DATA32)pPeb32->Ldr)->InMemoryOrderModuleList.Flink; pListEntry32 != &((PPEB_LDR_DATA32)pPeb32->Ldr)->InMemoryOrderModuleList; pListEntry32 = (PLIST_ENTRY32)pListEntry32->Flink)
			{
				PLDR_DATA_TABLE_ENTRY32 pTableEntry32 = CONTAINING_RECORD(pListEntry32, LDR_DATA_TABLE_ENTRY32, InMemoryOrderLinks);

				UNICODE_STRING ustr;
				RtlUnicodeStringInit(&ustr, (PWCH)pTableEntry32->BaseDllName.Buffer);
				UNICODE_STRING client;
				RtlUnicodeStringInit(&client, L"client.dll");
				UNICODE_STRING engine;
				RtlUnicodeStringInit(&engine, L"engine.dll");
				
				if (RtlCompareUnicodeString(&ustr, &client, TRUE) == 0)
				{
					DbgPrint("1. Base DLL Name: %wZ \n", ustr);
					DbgPrint("2. DLL Base Address %p \n", pTableEntry32->DllBase);
					DbgPrint("3. DLL Entry Point %p \n", pTableEntry32->EntryPoint);
					clientBaseAddress = pTableEntry32->DllBase;

				}

				if(RtlCompareUnicodeString(&ustr, &engine, TRUE) == 0)
				{
					DbgPrint("1. Base DLL Name: %wZ \n", ustr);
					DbgPrint("2. DLL Base Address %p \n", pTableEntry32->DllBase);
					DbgPrint("3. DLL Entry Point %p \n", pTableEntry32->EntryPoint);
					engineBaseAddress = pTableEntry32->DllBase;

				}

			}

			KeUnstackDetachProcess(&state32);

		}

	}
	return 0;

}