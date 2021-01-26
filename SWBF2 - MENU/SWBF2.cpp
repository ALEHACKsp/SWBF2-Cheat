#include <Windows.h>
#include <iostream>
#include <thread>
#include <dwmapi.h>

#pragma comment(lib, "dwmapi.lib")

#include "SWBF2 Menu.h"
#include "framework.h"
#include "directx.h"

LRESULT CALLBACK sublayerProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK windowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

HFONT newFont = CreateFontA(20, 0, 0, 0, 0, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, "Fixedsys");

LRESULT CALLBACK sublayerProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK windowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int cmdShow)
{
	std::thread updateAddressesThread(updateAddresses);
	std::thread damageIncreaseThread(damageIncrease);

	AllocConsole();
	FILE* stream;
	freopen_s(&stream, "CONOUT$", "w", stdout);

	//create window class
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = windowProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(hInstance, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)RGB(0, 0, 0);
	wc.lpszClassName = "Moonlight";
	wc.hIcon = static_cast<HICON>(LoadImage(hInstance, MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, 16, 16, 0));
	wc.hIconSm = static_cast<HICON> (LoadImage(hInstance, MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, 16, 16, 0));
	RegisterClassEx(&wc);

	//create window
	hWndMain = CreateWindowEx(NULL, "Moonlight", " ", WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, 400, 400, 800, 600, NULL, NULL, hInstance, NULL);

	//show window
	ShowWindow(hWndMain, cmdShow);

	WNDCLASSEX sublayerClass;
	ZeroMemory(&sublayerClass, sizeof(WNDCLASSEX));
	sublayerClass.cbSize = sizeof(WNDCLASSEX);
	sublayerClass.style = CS_HREDRAW | CS_VREDRAW;
	sublayerClass.lpfnWndProc = sublayerProc;
	sublayerClass.hInstance = GetModuleHandleA(nullptr);
	sublayerClass.hbrBackground = (HBRUSH)RGB(0, 0, 0);
	sublayerClass.lpszClassName = "sublayer";
	RegisterClassEx(&sublayerClass);

	hWndSubLayer = CreateWindowEx(WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_LAYERED, "sublayer", " ", WS_POPUP, 0, 0, 1920, 1080, NULL, NULL, GetModuleHandleA(nullptr), NULL);
	SetLayeredWindowAttributes(hWndSubLayer, RGB(0, 0, 0), 100, LWA_ALPHA);

	MARGINS margins = { -1 };
	DwmExtendFrameIntoClientArea(hWndSubLayer, &margins);

	ShowWindow(hWndSubLayer, SW_SHOWDEFAULT);
	UpdateWindow(hWndSubLayer);

	initD3D(hWndSubLayer);

	MSG message = { 0 };

	while (true)
	{
		POINT mouseCursor;
		GetCursorPos(&mouseCursor);
		ScreenToClient(hWndMain, &mouseCursor);

		if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
		{
			//translate
			TranslateMessage(&message);

			//dispatch
			DispatchMessage(&message);

			//quit
			if (message.message == WM_QUIT)
				break;

		}

		if (buttonUpdate)
		{
			if (!attachedToDriver)
			{
				SetRect(&attachedToDriverRect, dimensionRect.left + 20, dimensionRect.top + 40, dimensionRect.left + 200, dimensionRect.top + 60);
				DrawText(hdc, TEXT("[ ] attach to driver"), -1, &attachedToDriverRect, DT_NOCLIP);

			}

			if (attachedToDriver)
			{
				SetRect(&attachedToDriverRect, dimensionRect.left + 20, dimensionRect.top + 40, dimensionRect.left + 200, dimensionRect.top + 60);
				DrawText(hdc, TEXT("[X] attach to driver"), -1, &attachedToDriverRect, DT_NOCLIP);

			}

			if (!espChecked && attachedToDriver)
			{
				SetRect(&espCheckedRect, dimensionRect.left + 20, dimensionRect.top + 80, dimensionRect.left + 50, dimensionRect.top + 100);
				DrawText(hdc, TEXT("[ ] esp"), -1, &espCheckedRect, DT_NOCLIP);

			}

			if (espChecked && attachedToDriver)
			{
				SetRect(&espCheckedRect, dimensionRect.left + 20, dimensionRect.top + 80, dimensionRect.left + 50, dimensionRect.top + 100);
				DrawText(hdc, TEXT("[X] esp"), -1, &espCheckedRect, DT_NOCLIP);

			}

			if (!espLinesChecked && attachedToDriver && espChecked)
			{
				SetRect(&espLinesRect, dimensionRect.left + 80, dimensionRect.top + 80, dimensionRect.left + 110, dimensionRect.top + 100);
				DrawText(hdc, TEXT("[ ] lines"), -1, &espLinesRect, DT_NOCLIP);

			}

			if (espLinesChecked && attachedToDriver && espChecked)
			{
				SetRect(&espLinesRect, dimensionRect.left + 80, dimensionRect.top + 80, dimensionRect.left + 110, dimensionRect.top + 100);
				DrawText(hdc, TEXT("[X] lines"), -1, &espLinesRect, DT_NOCLIP);

			}

			if (!espBoxesChecked && attachedToDriver && espChecked)
			{
				SetRect(&espBoxRect, dimensionRect.left + 170, dimensionRect.top + 80, dimensionRect.left + 200, dimensionRect.top + 100);
				DrawText(hdc, TEXT("[ ] boxes"), -1, &espBoxRect, DT_NOCLIP);

			}

			if (espBoxesChecked && attachedToDriver && espChecked)
			{
				SetRect(&espBoxRect, dimensionRect.left + 170, dimensionRect.top + 80, dimensionRect.left + 200, dimensionRect.top + 100);
				DrawText(hdc, TEXT("[X] boxes"), -1, &espBoxRect, DT_NOCLIP);

			}
			buttonUpdate = false;
			allowClicking = true;

		}

		if (GetAsyncKeyState(VK_LBUTTON))
		{
			if (allowClicking)
			{
				if (mouseCursor.x > attachedToDriverRect.left && mouseCursor.x < attachedToDriverRect.right && mouseCursor.y > attachedToDriverRect.top && mouseCursor.y < attachedToDriverRect.bottom && allowClicking && !attachedToDriver)
				{
					processBaseAddress = getProcessBaseAddress();
					pebBaseAddress = getProcessPeb();
					
					printf("-> Process Base Address: 0x%p \n", processBaseAddress);
					printf("-> PEB Base Address: 0x%p \n", pebBaseAddress);

					attachedToDriver = true;
					allowClicking = false;
					buttonUpdate = true;
					Sleep(100);

				}

				if (mouseCursor.x > espCheckedRect.left && mouseCursor.x < espCheckedRect.right && mouseCursor.y > espCheckedRect.top && mouseCursor.y < espCheckedRect.bottom && allowClicking && attachedToDriver)
				{
					espChecked = (espChecked != true);

					allowClicking = false;
					buttonUpdate = true;
					Sleep(100);

				}

				if (mouseCursor.x > espLinesRect.left && mouseCursor.x < espLinesRect.right && mouseCursor.y > espLinesRect.top && mouseCursor.y < espLinesRect.bottom && allowClicking && attachedToDriver && espChecked)
				{
					espLinesChecked = (espLinesChecked != true);

					allowClicking = false;
					buttonUpdate = true;
					Sleep(100);

				}

				if (mouseCursor.x > espBoxRect.left && mouseCursor.x < espBoxRect.right && mouseCursor.y > espBoxRect.top && mouseCursor.y < espBoxRect.bottom && allowClicking && attachedToDriver && espChecked)
				{
					espBoxesChecked = (espBoxesChecked != true);

					allowClicking = false;
					buttonUpdate = true;
					Sleep(100);

				}

			}

		}
		GetClientRect(hWndMain, &dimensionRect);
		render();

	}
	endThreads = true;
	updateAddressesThread.join();
	damageIncreaseThread.join();
	clean();
	EndPaint(hWndMain, &ps);
	DestroyWindow(hWndMain);
	DestroyWindow(hWndSubLayer);
	DeleteDC(hdc);

}

LRESULT CALLBACK windowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:
	{
		hdc = BeginPaint(hWndMain, &ps);

		SelectObject(hdc, newFont);

		//fill window with white
		FillRect(hdc, &dimensionRect, (HBRUSH)RGB(0, 0, 0));

		//create title
		SetRect(&titleRect, dimensionRect.left, dimensionRect.top, dimensionRect.right, dimensionRect.top + 20);
		DrawTextA(hdc, "MOONLIGHT.GG | DRAW!#0627", -1, &titleRect, DT_NOCLIP | DT_CENTER);

		if (!attachedToDriver)
		{
			SetRect(&attachedToDriverRect, dimensionRect.left + 20, dimensionRect.top + 40, dimensionRect.left + 200, dimensionRect.top + 60);
			DrawText(hdc, TEXT("[ ] attach to driver"), -1, &attachedToDriverRect, DT_NOCLIP);

		}

		if (attachedToDriver)
		{
			SetRect(&attachedToDriverRect, dimensionRect.left + 20, dimensionRect.top + 40, dimensionRect.left + 200, dimensionRect.top + 60);
			DrawText(hdc, TEXT("[X] attach to driver"), -1, &attachedToDriverRect, DT_NOCLIP);

		}

		if (!espChecked && attachedToDriver)
		{
			SetRect(&espCheckedRect, dimensionRect.left + 20, dimensionRect.top + 80, dimensionRect.left + 200, dimensionRect.top + 100);
			DrawText(hdc, TEXT("[ ] esp"), -1, &espCheckedRect, DT_NOCLIP);

		}

		if (espChecked && attachedToDriver)
		{
			SetRect(&espCheckedRect, dimensionRect.left + 20, dimensionRect.top + 80, dimensionRect.left + 200, dimensionRect.top + 100);
			DrawText(hdc, TEXT("[X] esp"), -1, &espCheckedRect, DT_NOCLIP);

		}

		if (!espLinesChecked && attachedToDriver && espChecked)
		{
			SetRect(&espLinesRect, dimensionRect.left + 80, dimensionRect.top + 80, dimensionRect.left + 110, dimensionRect.top + 100);
			DrawText(hdc, TEXT("[ ] lines"), -1, &espLinesRect, DT_NOCLIP);

		}

		if (espLinesChecked && attachedToDriver && espChecked)
		{
			SetRect(&espLinesRect, dimensionRect.left + 80, dimensionRect.top + 80, dimensionRect.left + 110, dimensionRect.top + 100);
			DrawText(hdc, TEXT("[X] lines"), -1, &espLinesRect, DT_NOCLIP);

		}

		if (!espBoxesChecked && attachedToDriver && espChecked)
		{
			SetRect(&espBoxRect, dimensionRect.left + 170, dimensionRect.top + 80, dimensionRect.left + 200, dimensionRect.top + 100);
			DrawText(hdc, TEXT("[ ] boxes"), -1, &espBoxRect, DT_NOCLIP);

		}

		if (espBoxesChecked && attachedToDriver && espChecked)
		{
			SetRect(&espBoxRect, dimensionRect.left + 170, dimensionRect.top + 80, dimensionRect.left + 200, dimensionRect.top + 100);
			DrawText(hdc, TEXT("[X] boxes"), -1, &espBoxRect, DT_NOCLIP);

		}

	} break;

	case WM_DESTROY:
	{
		endThreads = true;
		clean();
		EndPaint(hWndMain, &ps);
		DestroyWindow(hWndMain);
		DestroyWindow(hWndSubLayer);
		DeleteDC(hdc);
		PostQuitMessage(0);
		return 0;

	} break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);

	}
	return 0;

}

LRESULT CALLBACK sublayerProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hWnd, message, wParam, lParam);

}