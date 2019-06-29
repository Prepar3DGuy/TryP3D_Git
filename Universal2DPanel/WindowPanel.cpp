#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include "SimConnect.h"
#include <strsafe.h>

bool quit = false;
bool internal = false;
HANDLE hSimConnect = NULL;

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

void Universal2DPanel()
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

		// 
		while (!quit)
		{
			SimConnect_CallDispatch(hSimConnect, MyDispatchProc, NULL);
			Sleep(1);
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
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
	// Check for argument that indicate start from P3D
	int argc;
	LPWSTR *argv;
	argv = CommandLineToArgvW(pCmdLine, &argc);
	if (argv != NULL && argc > 0)
	{
		if (lstrcmpW(argv[0], L"internal") == 0)
		{
			internal = true;
			MessageBoxW(NULL, L"Applicatoin was started by Prepar3D.\nIn case of debugging you can attach Visual Studio debugger to the process Universal2DPanel.exe using Debug->Attach to process", L"Universal2DPanel", MB_ICONINFORMATION | MB_OK | MB_APPLMODAL);
		}
	}
	LocalFree(argv);

		
	Universal2DPanel();

	return 0;
}