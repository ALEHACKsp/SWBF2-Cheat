#include "hook.h"
#include "cleaning.h"

using namespace driver;

extern "C" NTSTATUS DriverEntry(PVOID lpBaseAddress, DWORD32 size)
{
	//debug print
	DbgPrint("Driver Loaded! \n");

	cleaning::debug = true;
	cleaning::driver_timestamp = 0x5A20EB65;
	cleaning::driver_name = RTL_CONSTANT_STRING(L"frAQBc8Wsa1xVPfv");
	cleaning::lp_base_address = lpBaseAddress;
	DbgPrint("Base Address: 0x%p", lpBaseAddress);

	nullHook::callKernelFunction(&nullHook::hookHandler);

	return STATUS_SUCCESS;

}