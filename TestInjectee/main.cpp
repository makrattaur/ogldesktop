#include <Windows.h>

#include <utilityMacros.h>

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC dc = wParam == 0 ? BeginPaint(hwnd, &ps) : (HDC)wParam;

			static char *str = "Hello World !";
			TextOut(dc, 100, 100, str, strlen(str));

			if(wParam == 0)
				EndPaint(hwnd, &ps);

			break;
		}
        case WM_CLOSE:
		{
			DestroyWindow(hwnd);
			break;
		}
        case WM_DESTROY:
		{
            PostQuitMessage(0);
			break;
		}
        default:
		{
            return DefWindowProc(hwnd, msg, wParam, lParam);
		}
    }

    return 0;
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX wndClassEx;

	ZERO_STRUCT_WIN32(wndClassEx);
    wndClassEx.lpfnWndProc = WndProc;
    wndClassEx.hInstance = hInstance;
    wndClassEx.lpszClassName = "InjectionTestWindow";
	wndClassEx.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClassEx.lpszMenuName = NULL;
	wndClassEx.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wndClassEx.cbWndExtra = sizeof(LONG_PTR);
	RegisterClassEx(&wndClassEx);

	HWND window = CreateWindow("InjectionTestWindow",
		"Injection Test Window",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		500, 250,
		NULL, NULL,
		hInstance, NULL);

	ShowWindow(window, nCmdShow);
	UpdateWindow(window);


	MSG msg;
	while(GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}

