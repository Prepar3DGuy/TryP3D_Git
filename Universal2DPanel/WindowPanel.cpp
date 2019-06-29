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

	switch (pData->dwID)
	{
		case SIMCONNECT_RECV_ID_EVENT:
		{
			SIMCONNECT_RECV_EVENT *evt = (SIMCONNECT_RECV_EVENT*)pData;

			switch (evt->uEventID)
			{
				case EVENT_MENU_PANEL:
				{
					printf("\nMenu selected\n");
					break;
				}

				default:
				{
					printf("\nReceived unknown event: %d", evt->uEventID);
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
			printf("\nReceived ID: %d", pData->dwID);
			break;
		}
	}
}

void Universal2DPanel()
{
	HRESULT hr;

	if (SUCCEEDED(SimConnect_Open(&hSimConnect, "Universal2DPanel", NULL, 0, 0, 0)))
	{
		printf("\nConnected to Prepar3D!");

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

		printf("\nDisconnected from Prepar3D");
	}
	else
		printf("\nFailed to connect to Prepar3D");
}

int main(int argc, _TCHAR* argv[])
{
	// Check for argument that indicate start from P3D
	if (argc > 1 && lstrcmp(argv[1], L"internal") == 0) 
		internal = true;
		
	Universal2DPanel();

	return 0;
}