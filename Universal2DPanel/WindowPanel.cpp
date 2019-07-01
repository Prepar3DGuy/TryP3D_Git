#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include "SimConnect.h"
#include <strsafe.h>
#include <windowsx.h>


bool quit = false;
bool internal = false;
HANDLE hSimConnect = NULL;

HWND hwnd = NULL;
WNDCLASS wc = {};
MSG msg = {};
UINT_PTR IDTimer;	// Timer for periodically send event WM_TIMER to the window
HWND hButton = NULL;
HWND hEdit = NULL;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		if (FAILED(SetTimer(hwnd, IDTimer, 10, NULL)))
		{
			return -1;
		}
		return 0;

	case WM_DESTROY:
		KillTimer(hwnd, IDTimer);
		DestroyWindow(hEdit);
		DestroyWindow(hButton);
		PostQuitMessage(0);
		return 0;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		
		// Draw window here
		FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 0));

		EndPaint(hwnd, &ps);
	}
	return 0;

	case WM_TIMER:
		if (wParam == IDTimer)
		{
			//InvalidateRect(hwnd, NULL, FALSE);
		}
		return 0;

	case WM_SIZE:
		InvalidateRect(hwnd, NULL, FALSE);
		return 0;

	case WM_COMMAND:
	{
		if (HIBYTE(wParam) == BN_CLICKED)
		{
			if ((HWND)lParam == hButton)
			{
				WCHAR text[100], msg[100];
				GetWindowText(hEdit, text, 100);
				swprintf_s(msg, L"Hello %s!", text);
				MessageBoxW(NULL, msg, L"Universal2DPanel", MB_ICONWARNING | MB_OK | MB_APPLMODAL);
				OutputDebugStringW(L"Button Clicked\n");
			}
		}
	}

	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void MakeWindow()
{
	// Register the window class.
	const wchar_t CLASS_NAME[] = L"U2DPANEL";
	
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	// Get P3D main window handle (there is only one window with FS98MAIN window class inside the process)
	HWND P3DMainWindow = FindWindow(L"FS98MAIN", NULL);

	// (We can try to create window of FS98CHILD or FS98FLOAT window classes, but its message processing are by P3D logic.
	// I doesn't know how to add gauges to it.)

	// Create the window.
	hwnd = CreateWindowEx(
		0, // Optional window styles.
		CLASS_NAME,  // Window class
		L"Hello World Window",    // Window text
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_SYSMENU | WS_OVERLAPPED, // Window style
		20, 20, 200, 100, // Position and size 
		P3DMainWindow,   // Parent window  (This prevent the window to show icon on taskbar)
		NULL,       // Menu
		GetModuleHandle(NULL),  // Instance handle
		NULL        // Additional application data
	);

	if (hwnd == NULL)
	{
		OutputDebugStringW(L"Error to create\n");
		return;
	}

	// Some changes to the default window styles
	SetWindowLongPtr(hwnd, GWL_STYLE, GetWindowLongPtr(hwnd, GWL_STYLE) & ~WS_CAPTION); // hide window caption
	SetWindowLongPtr(hwnd, GWL_EXSTYLE, GetWindowLongPtr(hwnd, GWL_EXSTYLE) & ~WS_EX_WINDOWEDGE); // hide window edge

	// Create button
	hButton = CreateWindow(
		L"BUTTON",
		L"Say Hello",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		100,
		10,
		80,
		20,
		hwnd,
		NULL,
		GetModuleHandle(NULL),
		NULL);

	// Create text edit
	hEdit = CreateWindow(
		L"EDIT",
		NULL,
		WS_CHILD | WS_VISIBLE,
		10,
		10,
		80,
		20,
		hwnd,
		NULL,
		GetModuleHandle(NULL),
		NULL);
}

// SimConnect Events Groups
enum GROUP_ID
{
	GROUP_MENU
};

// SimConnect Events
enum EVENT_ID
{
	EVENT_MENU_PANEL
};

void CALLBACK MyDispatchProc(SIMCONNECT_RECV* pData, DWORD cbData, void *pContext)
{
	HRESULT hr;
	WCHAR msg[100];

	switch (pData->dwID)
	{
		case SIMCONNECT_RECV_ID_EVENT:
		{
			SIMCONNECT_RECV_EVENT *evt = (SIMCONNECT_RECV_EVENT*)pData;

			switch (evt->uEventID)
			{
				case EVENT_MENU_PANEL:
				{
					OutputDebugStringW(L"Menu selected\n");
					internal = !internal;
					ShowWindow(hwnd, internal ? SW_HIDE : SW_SHOW); //SW_SHOWNORMAL
					break;
				}

				default:
				{
					swprintf_s(msg, L"Received unknown event: %d\n", evt->uEventID);
					OutputDebugStringW(msg);
					break;
				}
			}
			
			break;
		}

		case SIMCONNECT_RECV_ID_QUIT:
		{
			quit = true;
			break;
		}

		default:
		{
			swprintf_s(msg, L"Received ID: %d\n", pData->dwID);
			OutputDebugStringW(msg);
			break;
		}
	}
}

DWORD WINAPI Universal2DPanel(LPVOID lpParam)
{
	HRESULT hr;

	if (SUCCEEDED(SimConnect_Open(&hSimConnect, "Universal2DPanel", NULL, 0, 0, 0)))
	{
		OutputDebugStringW(L"Connected to Prepar3D!\n");

		// Create private events
		hr = SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_MENU_PANEL);
		
		// Add menu item
		hr = SimConnect_MenuAddItem(hSimConnect, "Universal 2DPanel", EVENT_MENU_PANEL, NULL);

		// Sign up for the notifications
		hr = SimConnect_AddClientEventToNotificationGroup(hSimConnect, GROUP_MENU, EVENT_MENU_PANEL);

		hr = SimConnect_SetNotificationGroupPriority(hSimConnect, GROUP_MENU, SIMCONNECT_GROUP_PRIORITY_DEFAULT);

		// Just once for DLL
		SimConnect_CallDispatch(hSimConnect, MyDispatchProc, NULL);

		// Create hidden window
		MakeWindow();

		// Run the message loop.
		while (!quit)
		{
			if (GetMessage(&msg, NULL, 0, 0))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else if (msg.message = WM_QUIT) quit = true;

		}

		// Remove menu item
		hr = SimConnect_MenuDeleteItem(hSimConnect, EVENT_MENU_PANEL);

		hr = SimConnect_Close(hSimConnect);

		OutputDebugStringW(L"Disconnected from Prepar3D\n");
	}
	else
	{
		MessageBoxW(NULL, L"Failed to connect to Prepar3D.\nPrepar3D is not running or SimConnect interface is unavailable.\nApplication will be closed.", L"Universal2DPanel", MB_ICONERROR | MB_OK | MB_APPLMODAL);
		OutputDebugStringW(L"Failed to connect to Prepar3D\n");
	}

	return 0;
}

HANDLE hMyThread;
DWORD dwThreadId;

extern "C" __declspec(dllexport) void __stdcall DLLStart( void )
{
	internal = true;

	// Create thread for window and SimConnect messages processing outside of caller thread
	hMyThread = CreateThread(
		NULL,
		0,
		Universal2DPanel,
		NULL,
		0,
		&dwThreadId
	);

	if (hMyThread == NULL)
	{
		return;
	}

	return; 
}

extern "C" __declspec(dllexport) void __stdcall DLLStop( void )
{
	quit = true;
	if (hMyThread != NULL)
	{
		WaitForSingleObject(hMyThread, INFINITE);
	}
}