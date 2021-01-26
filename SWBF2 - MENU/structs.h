#pragma once
#include <Windows.h>

typedef struct memory
{
	//boolean
	BOOLEAN write;
	BOOLEAN read;
	BOOLEAN requestBase;
	BOOLEAN requestPeb;
	BOOLEAN write2;

	//process base address
	UINT64 processBaseAddress;

	//peb
	UINT64 peb;

	ULONG clientBaseAddress;
	ULONG engineBaseAddress;

	//rvpm & wvpm
	PVOID response;
	PVOID address;
	SIZE_T size;
	PVOID buffer;

	//module name
	const char* moduleName;

} memory, * pmemory;

struct viewMatrix
{
	float matrix[4][4];

};

struct vec4
{
	float x;
	float y;
	float z;
	float w;

	vec4 operator + (vec4 L)
	{
		return { x + L.x, y + L.y, z + L.z, w + L.z };

	}

	vec4 operator - (vec4 L)
	{
		return { x - L.x, y - L.y, z - L.z, w - L.z };

	}

	vec4 operator * (int L)
	{
		return { x * L, y * L, z * L, w * L };

	}

};

struct vec3
{
	float x;
	float y;
	float z;

	vec3 operator + (vec3 w)
	{
		return { x + w.x, y + w.y, z + w.z };

	}

	vec3 operator - (vec3 w)
	{
		return { x - w.x, y - w.y, z - w.z };

	}

	vec3 operator * (int w)
	{
		return { x * w, y * w, z * w };

	}

	void normalize()
	{
		while (y < -180)
		{
			y += 360;

		}

		while (y > 180)
		{
			y -= 360;

		}

		if (x > 89)
		{
			x = 89;

		}

		if (x < -89)
		{
			x = -89;

		}

	}

};

struct vec2
{
	float x;
	float y;

	vec2 operator + (vec2 w)
	{
		return { x + w.x, y + w.y };

	}

	vec2 operator - (vec2 w)
	{
		return { x - w.x, y - w.y };

	}

	vec2 operator * (int w)
	{
		return { x * w, y * w };

	}

	void normalize()
	{
		while (y < -180)
		{
			y += 360;

		}

		while (y > 180)
		{
			y -= 360;

		}

		if (x > 89)
		{
			x = 89;

		}

		if (x < -89)
		{
			x = -89;

		}

	}

	void normalizePitch()
	{
		if (x > 89)
		{
			x = 89;

		}

		if (x < -89)
		{
			x = -89;

		}

	}

};


