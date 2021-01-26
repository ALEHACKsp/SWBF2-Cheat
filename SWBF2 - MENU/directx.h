#pragma once
#include <Windows.h>
#include <windowsx.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <string>
#include <iostream>
#include <sstream>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

#include "func.h"

#define ARGB_TRANS 0x00000000
#define CUSTOMFVF (D3DFVF_XYZRHW| D3DFVF_DIFFUSE)

LPDIRECT3D9 d3dInterface;
LPDIRECT3DDEVICE9 d3dDevice;
LPDIRECT3DVERTEXBUFFER9 virtualBuffer;
VOID* pVoid;

bool initD3D(HWND hWnd)
{
	//create d3d interface
	d3dInterface = Direct3DCreate9(D3D_SDK_VERSION);

	//create present parameters
	D3DPRESENT_PARAMETERS d3dPP;
	ZeroMemory(&d3dPP, sizeof(d3dPP));
	d3dPP.Windowed = true;
	d3dPP.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dPP.hDeviceWindow = hWnd;
	d3dPP.BackBufferFormat = D3DFMT_X8R8G8B8;
	d3dPP.BackBufferWidth = 1920;
	d3dPP.BackBufferHeight = 1080;

	//create device
	d3dInterface->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dPP, &d3dDevice);

	d3dDevice->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);

	return true;

}

bool drawLine(float startX, float startY, float endX, float endY, D3DCOLOR color)
{
	ID3DXLine* LineL;
	D3DXCreateLine(d3dDevice, &LineL);

	D3DXVECTOR2 Line[2];
	Line[0] = { startX, startY };
	Line[1] = { endX, endY };
	LineL->SetWidth(2);
	LineL->Draw(Line, 2, color);
	LineL->Release();

	return true;

}

bool render(void)
{
	d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, ARGB_TRANS, 1.0f, 0);

	//begins
	d3dDevice->BeginScene();

	//select format 
	d3dDevice->SetFVF(CUSTOMFVF);

	if (attachedToDriver)
	{
		if (espChecked)
		{
			UINT64 localPlayerAddress = CHAIN_POINTER(processBaseAddress, { offsets::game::GAME_MANAGER, offsets::game::PLAYER_MANAGER, offsets::game::LOCAL_PLAYER });
			UINT64 playerArrayAddress = CHAIN_POINTER(processBaseAddress, { offsets::game::GAME_MANAGER, offsets::game::PLAYER_MANAGER, offsets::game::PLAYER_ARRAY });
			if (localPlayerAddress && playerArrayAddress)
			{
				for (int i = 0; i < 100; i++)
				{
					UINT64 localPlayerTeamID = RVPM<UINT64>(localPlayerAddress + offsets::entity::TEAM_ID);
					if (localPlayerTeamID > 3 || localPlayerTeamID < 0) continue;

					UINT64 enemyPlayer = RVPM<UINT64>(playerArrayAddress + i * 0x8);

					if (localPlayerAddress == enemyPlayer) continue;

					UINT64 enemyPlayerTeamID = RVPM<UINT64>(enemyPlayer + offsets::entity::TEAM_ID);
					if (enemyPlayerTeamID > 3 || enemyPlayerTeamID < 0) continue;
					if (localPlayerTeamID == enemyPlayerTeamID) continue;

					if (enemyPlayerTeamID != localPlayerTeamID)
					{
						UINT64 enemyPlayerHealthAddress = CHAIN_POINTER(enemyPlayer, { offsets::entity::SOLDIER, offsets::entity::HEALTH });
						float enemyPlayerHealth = RVPM<float>(enemyPlayerHealthAddress + offsets::entity::HEALTH2);

						if (enemyPlayerHealth < 1.0f || enemyPlayerHealth == 0.0f) continue;

						UINT64 enemyPositionAddress = CHAIN_POINTER(enemyPlayer, { offsets::entity::SOLDIER, offsets::entity::POSITION });
						vec3 enemyPosition = RVPM<vec3>(enemyPositionAddress + offsets::entity::POSITION2);
						UINT64 localPlayerPositionAddress = CHAIN_POINTER(localPlayerAddress, { offsets::entity::SOLDIER, offsets::entity::POSITION });
						vec3 localPlayerPosition = RVPM<vec3>(localPlayerPositionAddress + offsets::entity::POSITION2);

						vec3 enemyHeadPosition;
						enemyHeadPosition.x = enemyPosition.x;
						enemyHeadPosition.y = enemyPosition.y + 1.5;
						enemyHeadPosition.z = enemyPosition.z;

						int distanceTo = sqrt(pow(enemyPosition.x - localPlayerPosition.x, 2) + pow(enemyPosition.y - localPlayerPosition.y, 2) + pow(enemyPosition.z - localPlayerPosition.z, 2));

						if (distanceTo > 2500) continue;

						vec2 headScreenOut;
						vec2 screenOut;

						if (!w2s(enemyPosition, screenOut)) continue;
						if (!w2s(enemyHeadPosition, headScreenOut)) continue;

						float height = headScreenOut.y - screenOut.y;
						float width = height / 2.4f;

						if (espLinesChecked)
							drawLine(960, 1080, screenOut.x, screenOut.y, D3DCOLOR_ARGB(255, 255, 0, 0));

						if (espBoxesChecked)
						{
							drawLine(screenOut.x - (width / 1.5), screenOut.y, screenOut.x + (width / 1.5), screenOut.y, D3DCOLOR_ARGB(255, 255, 0, 0));
							drawLine(screenOut.x - (width / 1.5), screenOut.y + height, screenOut.x + (width / 1.5), screenOut.y + height, D3DCOLOR_ARGB(255, 255, 0, 0));
							drawLine(screenOut.x - (width / 1.5), screenOut.y, screenOut.x - (width / 1.5), screenOut.y + height, D3DCOLOR_ARGB(255, 255, 0, 0));
							drawLine(screenOut.x + (width / 1.5), screenOut.y, screenOut.x + (width / 1.5), screenOut.y + height, D3DCOLOR_ARGB(255, 255, 0, 0));

						}

					}

				}

			}

		}

		//ends
		d3dDevice->EndScene();

		d3dDevice->Present(NULL, NULL, NULL, NULL);

		return true;

	}

};

bool clean(void)
{
	virtualBuffer->Release();
	d3dDevice->Release();
	d3dInterface->Release();

	return true;
};