#include "hook.h"
#include "ntos.h"

#include "cleaning.h"
#include "utils.h"
#include "skCrypter.h"
//#include "ldisasm.h"

using namespace driver;

bool findProcessAgain = true;
bool findPebAgain = true;

bool nullHook::callKernelFunction(void* kernelFunctionAddress)
{
	auto dxgkrnlPath = skCrypt("\\SystemRoot\\System32\\drivers\\dxgkrnl.sys");
	auto dxgkrnlFunc = skCrypt("NtQueryCompositionSurfaceStatistics");

	if (!kernelFunctionAddress)
	{
		DbgPrint("Invalid function address! \n");
		return false;

	}

	//grabs function memory address
	//dxgkrnl.sys ->
		//NtQueryCompositionSurfaceStatistics           - valid
		//NtOpenCompositionSurfaceSectionInfo           - valid
		//NtOpenCompositionSurfaceSwapChainHandleInfo
		//NtOpenCompositionSurfaceDirtyRegion
	PVOID* function = reinterpret_cast<PVOID*>(getSystemModuleExport(dxgkrnlPath.decrypt(), dxgkrnlFunc.decrypt()));

	//checks to see if function memory address is valid
	if (!function)
	{
		DbgPrint("Module export failed! \n");
		return false;

	}

	BYTE origin[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	//mov rax, function - 0x48 0xB8 
	//lea rax, function - 0x48 0x8D 
	BYTE shellCode[] = 
	{ 
		0x48, 0xB8                                //mov rax

	};

	//jmp rax          - 0x48 0xFF 0xE0          
	//jmp rbx          - 0x48 0xFF 0xE3
	BYTE shellCodeEnd[] = 
	{  
		0xFF, 0xE0                         //call rax

	}; 

	//size_t lengthOfShellcode = ldisasm(shellCode, true);
	//DbgPrint("Shell Code Length %zu \n", lengthOfShellcode);

	//size_t lengthOfShellcodeEnd = ldisasm(shellCodeEnd, true);
	//DbgPrint("Shell Code End Length  %zu \n", lengthOfShellcodeEnd);

	//secure memory for hook
	RtlSecureZeroMemory(&origin, sizeof(origin));
	//copy original function (mov) to memory address of shellcode (size of shellcode)
	memcpy((PVOID)((ULONG_PTR)origin), &shellCode, sizeof(shellCode));
	//covert function to uintptr_t
	uintptr_t hookAddress = reinterpret_cast<uintptr_t>(kernelFunctionAddress);
	//copy orginal function + size of shellcode to hookAddress memory address (size of void*)
	memcpy((PVOID)((ULONG_PTR)origin + sizeof(shellCode)), &hookAddress, sizeof(void*));
	//copy original function + size of shellcode + size of void* to shellCodeEnd memory address (size of shellCodeEnd)
	memcpy((PVOID)((ULONG_PTR)origin + sizeof(shellCode) + sizeof(void*)), &shellCodeEnd, sizeof(shellCodeEnd));
	
	//essentially detour hook function
	writeToReadOnlyMemory(function, &origin, sizeof(origin));

	DbgPrint("Successfully hooked! \n");

	cleaning::clean_traces();

	dxgkrnlPath.clear();
	dxgkrnlFunc.clear();

	return true;

}

NTSTATUS nullHook::hookHandler(PVOID calledParameter)
{
	memory* instructions = (memory*)calledParameter;
	
	if (instructions->requestBase)
	{
		if (findProcessAgain)
		{
			DbgPrint("Base request recieved! \n");
			findProcess();
			findClientBaseAddress();
			//findClientBaseAddressX86();
			findProcessAgain = false;

		}
		instructions->processBaseAddress = (UINT64)processBaseAddress;
		//instructions->clientBaseAddress = (ULONG)clientBaseAddress;
		//instructions->engineBaseAddress = (ULONG)engineBaseAddress;

	}

	if (instructions->requestPeb)
	{
		if (findPebAgain)
		{
			DbgPrint("PEB request recieved! \n");

			findPebAgain = false;

		}
		instructions->peb = (UINT64)pPeb64;

	}

	if (instructions->read)
	{
		DbgPrint("Read request received! \n");
		RVPM(instructions->address, instructions->response, instructions->size);
		
	}

	if (instructions->write)
	{
		DbgPrint("Write request received! \n");
		WVPM(instructions->address, instructions->buffer, instructions->size);

	}

	if (instructions->write2)
	{
		DbgPrint("Write request received \n");
		WVPM2(instructions->address, instructions->buffer, instructions->size);

	}
	return STATUS_SUCCESS;

}
