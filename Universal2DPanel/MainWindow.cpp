#ifndef UNICODE
#define UNICODE
#endif


//#include "basewin.h"

#include "MainWindow.h"

float DPIScale::scaleX = 1.0f;
float DPIScale::scaleY = 1.0f;



void MainWindow::OnLButtonDown(int pixelX, int pixelY, DWORD flags)
{
	SetCapture(m_hwnd);
	m_ellipse.point = ptMouse = DPIScale::PixelsToDips(pixelX, pixelY);
	m_ellipse.radiusX = m_ellipse.radiusY = 1.0f;
	InvalidateRect(m_hwnd, NULL, FALSE);
}

void MainWindow::OnMouseMove(int pixelX, int pixelY, DWORD flags)
{
	if (flags & MK_LBUTTON)
	{
		const D2D1_POINT_2F dips = DPIScale::PixelsToDips(pixelX, pixelY);

		const float dx = dips.x - ptMouse.x;
		const float dy = dips.y - ptMouse.y;
		const float radius = min(fabsf(dx), fabsf(dy)) / 2;
		const float xc = ptMouse.x + radius*(dx / fabsf(dx));
		const float yc = ptMouse.y + radius*(dy / fabsf(dy));

		m_ellipse = D2D1::Ellipse(D2D1::Point2F(xc, yc), radius, radius);

		InvalidateRect(m_hwnd, NULL, FALSE);
	}
}

void MainWindow::OnLButtonUp()
{
	ReleaseCapture();
}

HRESULT MainWindow::CreateGraphicsResources()
{
	HRESULT hr = S_OK;
	if (pRenderTarget == NULL)
	{
		RECT rc;
		GetClientRect(m_hwnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

		hr = pFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(m_hwnd, size),
			&pRenderTarget);

		if (SUCCEEDED(hr))
		{
			const D2D1_COLOR_F color = D2D1::ColorF(1.0f, 1.0f, 0);
			hr = pRenderTarget->CreateSolidColorBrush(color, &pBrush);

			if (SUCCEEDED(hr))
			{
				const D2D1_COLOR_F blackColor = D2D1::ColorF(0.0f, 0.0f, 0.0f);
				hr = pRenderTarget->CreateSolidColorBrush(blackColor, &m_pStroke);

				if (SUCCEEDED(hr))
				{
					const D2D1_COLOR_F greenColor = D2D1::ColorF(0.0f, 1.0f, 0.0f, 0.5f);
					hr = pRenderTarget->CreateSolidColorBrush(greenColor, &m_pInfoBrush);
				}
			}
		}

		if (SUCCEEDED(hr))
		{
			hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
				__uuidof(IDWriteFactory),
				reinterpret_cast<IUnknown**>(&pIDWriteFactory));
		}

		if (SUCCEEDED(hr))
		{
			hr = pIDWriteFactory->CreateTextFormat(
				L"ISOCPEUR", //L"Arial",
				NULL,
				DWRITE_FONT_WEIGHT_NORMAL,
				DWRITE_FONT_STYLE_NORMAL,
				DWRITE_FONT_STRETCH_NORMAL,
				12.0f * 96.0f / 72.0f,
				L"en-US",
				&pITextFormat
			);
			if (FAILED(hr))
			{
				hr = pIDWriteFactory->CreateTextFormat(
					L"Arial",
					NULL,
					DWRITE_FONT_WEIGHT_NORMAL,
					DWRITE_FONT_STYLE_NORMAL,
					DWRITE_FONT_STRETCH_NORMAL,
					10.0f * 96.0f / 72.0f,
					L"en-US",
					&pITextFormat
				);
			}
		}

		if (SUCCEEDED(hr))
		{
			hr = pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::Black),
				&pIBlackBrush
			);
		}

	}
	return hr;
}

void MainWindow::DiscardGraphicsResources()
{
	SafeRelease(&pRenderTarget);
	SafeRelease(&pBrush);
}

void MainWindow::DrawClockHand(float fHandLength, float fAngle, float fStrokeWidth)
{
	pRenderTarget->SetTransform(
		D2D1::Matrix3x2F::Rotation(fAngle, m_ellipse.point)
	);

	// endPoint defines one end of the hand.
	D2D_POINT_2F endPoint = D2D1::Point2F(
		m_ellipse.point.x,
		m_ellipse.point.y - (m_ellipse.radiusY * fHandLength)
	);

	// Draw a line from the center of the ellipse to endPoint.
	pRenderTarget->DrawLine(
		m_ellipse.point,
		endPoint,
		m_pStroke,
		fStrokeWidth
	);
}

void MainWindow::RenderScene()
{
	pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::SkyBlue));

	pRenderTarget->FillEllipse(m_ellipse, pBrush);
	pRenderTarget->DrawEllipse(m_ellipse, m_pStroke);

	// Draw hands
	SYSTEMTIME time;
	GetLocalTime(&time);

	// 60 minutes = 30 degrees, 1 minutes = 0.5 degree
	const float fHourAngle = (360.0f / 12) * (time.wHour) + (time.wMinute * 0.5f);
	// 60 minutes = 360 degrees, 1 minutes = 6 degree, 60 seconds = 6 degrees, 1 seconds = 0.1 degree
	const float fMinuteAngle = (360.0f / 60) * (time.wMinute) + (time.wSecond * 0.1f);
	// 60 seconds = 360 degrees, 1 seconds = 6 degree, 1 msec = 6/1000 degrees
	const float fSecondAngle = (360.0f / 60) * (time.wSecond) + (time.wMilliseconds * 6.0f / 1000);

	DrawClockHand(0.6f, fHourAngle, 6);
	DrawClockHand(0.85f, fMinuteAngle, 4);
	DrawClockHand(0.9f, fSecondAngle, 2);

	// Restore the identity transformation
	pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

	// Output Time Stamp
	if (bTimeStamp)
	{
		WCHAR sCurrentTime[10];
		swprintf_s(sCurrentTime, L"%2d:%02d:%02d", time.wHour, time.wMinute, time.wSecond);

		m_rectTimeStamp = D2D1::RectF(ptMouse.x, ptMouse.y, ptMouse.x + 55.0f, ptMouse.y - 20.0f); // size ???
		pRenderTarget->FillRectangle(m_rectTimeStamp, m_pInfoBrush);

		// Text output via DirectWrite
		pRenderTarget->DrawText(
			sCurrentTime,
			(UINT32)wcslen(sCurrentTime),
			pITextFormat,
			m_rectTimeStamp,
			pIBlackBrush
		);
	}
}

void MainWindow::OnPaint()
{
	HRESULT hr = CreateGraphicsResources();
	if (SUCCEEDED(hr))
	{
		PAINTSTRUCT ps;
		BeginPaint(m_hwnd, &ps);

		pRenderTarget->BeginDraw();

		RenderScene();

		hr = pRenderTarget->EndDraw();
		if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
		{
			DiscardGraphicsResources();
		}
		EndPaint(m_hwnd, &ps);
	}
}

void MainWindow::Resize()
{
	if (pRenderTarget != NULL)
	{
		RECT rc;
		GetClientRect(m_hwnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

		pRenderTarget->Resize(size);
		InvalidateRect(m_hwnd, NULL, FALSE);
	}
}

//int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
//{
//	MainWindow win;
//
//	if (!win.Create(L"Circle", WS_OVERLAPPEDWINDOW))
//	{
//		return 0;
//	}
//
//	ShowWindow(win.Window(), nCmdShow);
//
//	// Run the message loop.
//
//	MSG msg = {};
//	while (GetMessage(&msg, NULL, 0, 0))
//	{
//		TranslateMessage(&msg);
//		DispatchMessage(&msg);
//	}
//
//	return 0;
//}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WCHAR msg[32];

	switch (uMsg)
	{
	case WM_CREATE:
		if (FAILED(D2D1CreateFactory(
			D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory)))
		{
			return -1;  // Fail CreateWindowEx.
		}
		DPIScale::Initialize(pFactory);
		if (FAILED(SetTimer(m_hwnd, m_IDTimer, 10, NULL)))
		{
			return -1;
		}
		return 0;

	case WM_DESTROY:
		KillTimer(m_hwnd, m_IDTimer);
		DiscardGraphicsResources();
		SafeRelease(&pFactory);
		PostQuitMessage(0);
		return 0;

	case WM_PAINT:
		OnPaint();
		return 0;

	case WM_TIMER:
		if (wParam == m_IDTimer)
		{
			//OnPaint();
			InvalidateRect(m_hwnd, NULL, FALSE);
		}
		return 0;

	case WM_LBUTTONDOWN:
		OnLButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (DWORD)wParam);
		return 0;

	case WM_LBUTTONUP:
		OnLButtonUp();
		return 0;

	case WM_MOUSEMOVE:
		OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (DWORD)wParam);
		if (wParam == 0)
		{
			mouseTrack.OnMouseMove(m_hwnd);
			if (bTimeStamp)
			{
				bTimeStamp = FALSE;
				InvalidateRect(m_hwnd, NULL, FALSE);
			}
		}
		return 0;

	case WM_MOUSELEAVE:
		if (bTimeStamp)
		{
			bTimeStamp = FALSE;
			InvalidateRect(m_hwnd, NULL, FALSE);
		}
		mouseTrack.Reset(m_hwnd);
		return 0;

	case WM_MOUSEHOVER:
		if (!bTimeStamp)
		{
			bTimeStamp = TRUE;
			ptMouse = DPIScale::PixelsToDips(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			InvalidateRect(m_hwnd, NULL, FALSE);
		}
		mouseTrack.Reset(m_hwnd);
		return 0;

	case WM_SIZE:
		Resize();
		return 0;

		// Keyboard input
	case WM_SYSKEYDOWN:
		swprintf_s(msg, L"WM_SYSKEYDOWN: 0x%x\n", (UINT)wParam);
		OutputDebugString(msg);
		break;

	case WM_SYSCHAR:
		swprintf_s(msg, L"WM_SYSCHAR: %c\n", (wchar_t)wParam);
		OutputDebugString(msg);
		break;

	case WM_SYSKEYUP:
		swprintf_s(msg, L"WM_SYSKEYUP: 0x%x\n", (UINT)wParam);
		OutputDebugString(msg);
		break;

	case WM_KEYDOWN:
		swprintf_s(msg, L"WM_KEYDOWN: 0x%x\n", (UINT)wParam);
		OutputDebugString(msg);
		if (wParam == VK_ESCAPE)
		{
			if (SUCCEEDED(DestroyWindow(m_hwnd)))
			{
				swprintf_s(msg, L"Send DestroyWindow message.\n");
				OutputDebugString(msg);
			}
		}
		break;

	case WM_KEYUP:
		swprintf_s(msg, L"WM_KEYUP: 0x%x\n", (UINT)wParam);
		OutputDebugString(msg);
		break;

	case WM_CHAR:
		swprintf_s(msg, L"WM_CHAR: %c\n", (wchar_t)wParam);
		OutputDebugString(msg);
		break;
	}
	return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}