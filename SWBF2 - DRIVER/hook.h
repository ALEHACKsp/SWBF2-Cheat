#pragma once
#include "mem.h" 

namespace nullHook
{
	bool callKernelFunction(void* kernelFunctionAddress);
	NTSTATUS hookHandler(PVOID calledParameter);

}