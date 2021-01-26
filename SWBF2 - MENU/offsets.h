#pragma once
#include <Windows.h>

namespace offsets
{
	namespace game
	{
		UINT64 GAME_MANAGER = 0x3DD7948;
		//48 89 15 ? ? ? ? 48 89 CB

		UINT64 PLAYER_MANAGER = 0x58;
		UINT64 PLAYER_ARRAY = 0x768;
		UINT64 LOCAL_PLAYER = 0x568;

	}

	namespace entity
	{
		UINT64 SOLDIER = 0x210;
		UINT64 TEAM_ID = 0x58;
		UINT64 HEALTH = 0x2C8;
		UINT64 HEALTH2 = 0x20;
		UINT64 POSITION = 0x758;
		UINT64 POSITION2 = 0x20;
		UINT64 NAME = 0x68;

	}

	namespace view
	{
		UINT64 GAME_RENDERER = 0x3FFBE10;
		//48 89 05 ? ? ? ? 4C 8B 00 

		UINT64 RENDER_VIEW = 0x538;
		UINT64 VIEW_MATRIX = 0x430;
		UINT64 YAW_ROLL_PITCH = 0x560;
		UINT64 PITCH = 0x55E;
		UINT64 YAW = 0x566;
	}

	namespace misc
	{
		UINT64 DAMAGE = 0x8367B0B;
		//8B 8A 50 01 00 00 49 8B 50 20 E9
		UINT64 SPREAD = 0x79C39F0;
		//43 0F 10 04 D9 0F 11 2 F2 43 0F 10 4C D9 10 F2 0F
		UINT64 GAME_TIME_SETTINGS = 0x3AEBBE8;
		//48 89 05 ? ? ? ? ? ? ? ? C7 40 ? ? ? ? ? ? ? ? ? ? 8B 43 18
		UINT64 FIRST_TYPE_INFO = 0x3D05F08;
		//48 8B 05 ? ? ? ? ? ? ? ? 48 89 41 08 48 89 0D
		UINT64 RECOIL = 0x7EEE11E;
		//E8 ? ? ? ? F2 0F 10 86 A8 01 00 00 8B86 7C 01 00 00

	}

};
