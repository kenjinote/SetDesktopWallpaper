#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <windows.h>

TCHAR szClassName[] = TEXT("Window");

enum WallpaperStyle
{
	Tile,
	Center,
	Stretch,
	Fit,
	Fill
};

HRESULT SetDesktopWallpaper(PWSTR pszFile, WallpaperStyle style)
{
	HKEY hKey = NULL;
	HRESULT hr = HRESULT_FROM_WIN32(RegOpenKeyEx(HKEY_CURRENT_USER,
		L"Control Panel\\Desktop", 0, KEY_READ | KEY_WRITE, &hKey));
	if (SUCCEEDED(hr))
	{
		PWSTR pszWallpaperStyle;
		PWSTR pszTileWallpaper;
		switch (style)
		{
		case Tile:
			pszWallpaperStyle = L"0";
			pszTileWallpaper = L"1";
			break;
		case Center:
			pszWallpaperStyle = L"0";
			pszTileWallpaper = L"0";
			break;
		case Stretch:
			pszWallpaperStyle = L"2";
			pszTileWallpaper = L"0";
			break;
		case Fit: // (Windows 7 and later)
			pszWallpaperStyle = L"6";
			pszTileWallpaper = L"0";
			break;
		case Fill: // (Windows 7 and later)
			pszWallpaperStyle = L"10";
			pszTileWallpaper = L"0";
			break;
		}
		DWORD cbData = lstrlen(pszWallpaperStyle) * sizeof(*pszWallpaperStyle);
		hr = HRESULT_FROM_WIN32(RegSetValueEx(hKey, L"WallpaperStyle", 0, REG_SZ,
			reinterpret_cast<const BYTE *>(pszWallpaperStyle), cbData));
		if (SUCCEEDED(hr))
		{
			cbData = lstrlen(pszTileWallpaper) * sizeof(*pszTileWallpaper);
			hr = HRESULT_FROM_WIN32(RegSetValueEx(hKey, L"TileWallpaper", 0, REG_SZ,
				reinterpret_cast<const BYTE *>(pszTileWallpaper), cbData));
		}
		RegCloseKey(hKey);
	}
	if (SUCCEEDED(hr))
	{
		if (!SystemParametersInfo(SPI_SETDESKWALLPAPER, 0,
			static_cast<PVOID>(pszFile),
			SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE))
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
		}
	}
	return hr;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND hCheck[5];
	switch (msg)
	{
	case WM_CREATE:
		hCheck[0]= CreateWindow(TEXT("BUTTON"), TEXT("Tile"), WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 10, 10, 256, 32, hWnd, (HMENU)Tile, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hCheck[1] = CreateWindow(TEXT("BUTTON"), TEXT("Center"), WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 10, 50, 256, 32, hWnd, (HMENU)Center, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hCheck[2] = CreateWindow(TEXT("BUTTON"), TEXT("Stretch"), WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 10, 90, 256, 32, hWnd, (HMENU)Stretch, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hCheck[3] = CreateWindow(TEXT("BUTTON"), TEXT("Fit"), WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 10, 130, 256, 32, hWnd, (HMENU)Fit, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hCheck[4] = CreateWindow(TEXT("BUTTON"), TEXT("Fill"), WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 10, 170, 256, 32, hWnd, (HMENU)Fill, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		SendMessage(hCheck[3], BM_SETCHECK, BST_CHECKED, 0);
		DragAcceptFiles(hWnd, TRUE);
		break;
	case WM_DROPFILES:
		{
			WCHAR szFileName[MAX_PATH];
			HDROP hDrop = (HDROP)wParam;
			DragQueryFileW(hDrop, 0, szFileName, sizeof(szFileName));
			DragFinish(hDrop);
			for (auto item : hCheck)
			{
				if (SendMessage(item, BM_GETCHECK, 0, 0))
				{
					SetDesktopWallpaper(szFileName, (WallpaperStyle)GetDlgCtrlID(item));
					break;
				}
			}
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR pCmdLine, int nCmdShow)
{
	MSG msg;
	WNDCLASS wndclass = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,
		0,
		hInstance,
		0,
		LoadCursor(0,IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1),
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	HWND hWnd = CreateWindow(
		szClassName,
		TEXT("ドロップされた画像をデスクトップの背景に設定する"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		0,
		0,
		hInstance,
		0
	);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}
