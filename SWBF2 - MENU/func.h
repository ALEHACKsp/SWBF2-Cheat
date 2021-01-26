#pragma once
#include <Windows.h>
#include <iostream>
#include <vector>
#include <TlHelp32.h>

#include "structs.h"
#include "offsets.h"

// == DATA 

long double PI = 3.14159;
#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

// == DATA



// == VARIABLES

UINT64 localPlayerAddress = 0;
UINT64 processBaseAddress = 0;
UINT64 pebBaseAddress = 0;
UINT64 gameManagerAddress = 0;
UINT64 playerManagerAddress = 0;
UINT64 playerArrayAddress = 0;
UINT64 gameRendererAddress = 0;
UINT64 renderViewAddress = 0;
UINT64 viewMatrixAddress = 0;

// == VARIABLES




// == HWNDS

HWND hWndMain;
HWND hWndSubLayer;

// == HWNDS





// == RECTS

RECT dimensionRect; //window dimensions
RECT titleRect; //title location
RECT attachedToDriverRect; //attach to driver button location
RECT espCheckedRect; //esp checked location
RECT processBaseAddressRect; //process base address debug info location
RECT pebBaseAddressRect; //peb address debug info location
RECT localPlayerAddressRect; //local player address debug info location
RECT espLinesRect;
RECT espBoxRect;

// == RECTS





// == PAINTSTRUCTS

PAINTSTRUCT ps;

// == PAINSTRUCTS





// == HDCS

HDC hdc;

//== HDCS




// == TOGGLES

bool allowClicking = true;
bool buttonUpdate = false;
bool attachedToDriver = false;
bool espChecked = false;
bool espLinesChecked = false;
bool espBoxesChecked = false; 

// == TOGGLES




// == THREADS

bool beginThreads = true;
bool endThreads = false;

// == THREADS

template <typename ... Arg>UINT64 callHook(const Arg ... args)
{
	LoadLibrary("user32.dll");

	void* hookedFunction = GetProcAddress(LoadLibrary("win32u.dll"), "NtQueryCompositionSurfaceStatistics");

	auto func = static_cast<UINT64(_stdcall*)(Arg...)>(hookedFunction);

	return func(args ...);

}

UINT64 getProcessBaseAddress()
{
	memory instructions = { 0 };
	ZeroMemory(&instructions, sizeof(instructions));
	instructions.requestBase = TRUE;
	instructions.read = FALSE;
	instructions.write = FALSE;
	instructions.requestPeb = FALSE;
	instructions.write2 = FALSE;
	callHook(&instructions);

	UINT64 base = NULL;
	base = instructions.processBaseAddress;
	return base;

}

UINT64 getProcessPeb()
{
	memory instructions = { 0 };
	ZeroMemory(&instructions, sizeof(instructions));
	instructions.requestBase = FALSE;
	instructions.read = FALSE;
	instructions.write = FALSE;
	instructions.requestPeb = TRUE;
	instructions.write2 = FALSE;
	callHook(&instructions);

	UINT64 output = NULL;
	output = instructions.peb;
	return output;

}

template<typename T> T RVPM(uintptr_t address)
{
	T response{};
	memory instructions = { 0 };
	ZeroMemory(&instructions, sizeof(instructions));
	instructions.requestBase = FALSE;
	instructions.requestPeb = FALSE;
	instructions.read = TRUE;
	instructions.write = FALSE;
	instructions.write2 = FALSE;
	instructions.address = (PVOID)address;
	instructions.size = sizeof(T);
	instructions.response = &response;
	callHook(&instructions);

	return (T)response;

}

template<typename T> T WVPM(uintptr_t address, int buffer)
{
	memory instructions;
	ZeroMemory(&instructions, sizeof(instructions));
	instructions.requestBase = FALSE;
	instructions.requestPeb = FALSE;
	instructions.read = FALSE;
	instructions.write = TRUE;
	instructions.write2 = FALSE;
	instructions.address = (PVOID)address;
	instructions.buffer = (PVOID)buffer;
	instructions.size = sizeof(T);
	callHook(&instructions);

	return (T)true;

}

template<typename T> T WVPM2(uintptr_t address, PVOID buffer)
{
	memory instructions;
	ZeroMemory(&instructions, sizeof(instructions));
	instructions.requestBase = FALSE;
	instructions.requestPeb = FALSE;
	instructions.read = FALSE;
	instructions.write = FALSE;
	instructions.write2 = TRUE;
	instructions.address = (PVOID)address;
	instructions.buffer = (PVOID)buffer;
	instructions.size = sizeof(T);
	callHook(&instructions);

	return (T)true;

}

bool WVPM3(uintptr_t address, BYTE* buffer, SIZE_T size)
{
	memory instructions;
	ZeroMemory(&instructions, sizeof(instructions));
	instructions.requestBase = FALSE;
	instructions.requestPeb = FALSE;
	instructions.read = FALSE;
	instructions.write = FALSE;
	instructions.write2 = TRUE;
	instructions.address = (PVOID)address;
	instructions.buffer = (PVOID)buffer;
	instructions.size = size;
	callHook(&instructions);

	return true;

}
template<typename T> T WVPM_VEC2_FLOAT(uintptr_t address, vec2 buffer)
{
	WVPM2<float>(address, buffer.x);
	WVPM2<float>(address + 0x4, buffer.y);

}


UINT64 CHAIN_POINTER(UINT64 address, std::vector<UINT64>offsets)
{
	UINT64 sumAddress = address;
	int i = 0;

	while (i < offsets.size())
	{
		UINT64 temp = RVPM<UINT64>(sumAddress + offsets[i]);

		sumAddress = temp;

		i += 1;

	}
	return sumAddress;

}

void nop(UINT64 address, UINT64 size)
{
	BYTE* nopArray = new byte[size];
	memset(nopArray, 0x90, size);

	WVPM3(address, nopArray, size);

}

bool w2s(vec3 position, OUT vec2& screen)
{
	UINT64 renderViewAddress = CHAIN_POINTER(processBaseAddress, { offsets::view::GAME_RENDERER, offsets::view::RENDER_VIEW });

	if (!renderViewAddress)
		return false;

	viewMatrix view = RVPM<viewMatrix>(renderViewAddress + offsets::view::VIEW_MATRIX);

	vec4 clipCoordinates;
	clipCoordinates.x = position.x * view.matrix[0][0] + position.y * view.matrix[1][0] + position.z * view.matrix[2][0] + view.matrix[3][0];
	clipCoordinates.y = position.x * view.matrix[0][1] + position.y * view.matrix[1][1] + position.z * view.matrix[2][1] + view.matrix[3][1];
	clipCoordinates.w = position.x * view.matrix[0][3] + position.y * view.matrix[1][3] + position.z * view.matrix[2][3] + view.matrix[3][3];

	if (clipCoordinates.w < 0.1f)
		return false;

	int mX = 1920 / 2;
	int mY = 1080 / 2;

	float x = (mX + mX * clipCoordinates.x / clipCoordinates.w);
	float y = (mY - mY * clipCoordinates.y / clipCoordinates.w);

	screen.x = x;
	screen.y = y;

	return true;

}

void updateAddresses()
{
	UINT64 oGameManagerAddress = 0;
	UINT64 oPlayerManagerAddress = 0;
	UINT64 oPlayerArrayAddress = 0;
	UINT64 oLocalPlayerAddress = 0;
	UINT64 oGameRenderer = 0;
	UINT64 oRenderView = 0;
	UINT64 oViewMatrix = 0;

	while (beginThreads && !endThreads)
	{
		if (attachedToDriver)
		{
			UINT64 localPlayerAddress = CHAIN_POINTER(processBaseAddress, { offsets::game::GAME_MANAGER, offsets::game::PLAYER_MANAGER, offsets::game::LOCAL_PLAYER });
			UINT64 playerArrayAddress = CHAIN_POINTER(processBaseAddress, { offsets::game::GAME_MANAGER, offsets::game::PLAYER_MANAGER, offsets::game::PLAYER_ARRAY });
			UINT64 renderViewAddress = CHAIN_POINTER(processBaseAddress, { offsets::view::GAME_RENDERER, offsets::view::RENDER_VIEW });

			if (gameManagerAddress != oGameManagerAddress || playerManagerAddress != oPlayerManagerAddress || playerArrayAddress != oPlayerArrayAddress || localPlayerAddress != oLocalPlayerAddress || gameRendererAddress != oGameRenderer || renderViewAddress != oRenderView )
			{
				system("cls");
				printf("-> Process Base Address: 0x%p \n", processBaseAddress);
				printf("-> PEB Base Address: 0x%p \n", pebBaseAddress);
				printf("-> Player Array Address: 0x%p \n", playerArrayAddress);
				printf("-> Local Player Address: 0x%p \n", localPlayerAddress);
				printf("-> Render View Address 0x%p \n", renderViewAddress);

			}
			oGameManagerAddress = gameManagerAddress;
			oPlayerManagerAddress = playerManagerAddress;
			oPlayerArrayAddress = playerArrayAddress;
			oLocalPlayerAddress = localPlayerAddress;
			oGameRenderer = gameRendererAddress;
			oRenderView = renderViewAddress;

		}

	}
	Sleep(1);

}

void damageIncrease()
{
	while (beginThreads && !endThreads)
	{
		if (attachedToDriver)
		{
			UINT64 localPlayerAddress = CHAIN_POINTER(processBaseAddress, { offsets::game::GAME_MANAGER, offsets::game::PLAYER_MANAGER, offsets::game::LOCAL_PLAYER });
			UINT64 localPlayerHealthAddress = CHAIN_POINTER(localPlayerAddress, { offsets::entity::SOLDIER, offsets::entity::HEALTH });
			float localPlayerHealth = RVPM<float>(localPlayerHealthAddress + offsets::entity::HEALTH2);

			if (localPlayerHealth)
			{
				nop(processBaseAddress + offsets::misc::SPREAD, 5);
				nop(processBaseAddress + offsets::misc::RECOIL, 5);

				BYTE* shellcode = new byte[6]{ 0xB9, 0x00, 0x00, 0x00, 0x00, 0x90 };
				int newDamage = 18;
				memcpy(shellcode + 1, &newDamage, 4);
				WVPM3(processBaseAddress + offsets::misc::DAMAGE, shellcode, 6);
				delete[] shellcode;

			}

		}

	}

}
