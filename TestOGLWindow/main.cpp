#include <Windows.h>

#include <GL/GL.h>
#include <GL/GLU.h>
#include <iostream>

#include <utilityMacros.h>

#include "dwmtricks.h"


void FillPixelFormatDesc(PIXELFORMATDESCRIPTOR &pfd)
{
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;

	// interleaved in the struct
	pfd.cRedBits = 0; pfd.cGreenBits = 0; pfd.cBlueBits = 0; pfd.cAlphaBits = 8;
	pfd.cRedShift = 0; pfd.cGreenShift = 0; pfd.cBlueShift = 0; pfd.cAlphaShift = 0;

	pfd.cAccumBits = 0;
	pfd.cAccumRedBits = 0; pfd.cAccumGreenBits = 0; pfd.cAccumBlueBits = 0; pfd.cAccumAlphaBits = 0;

	pfd.cDepthBits = 32;
	pfd.cStencilBits = 8;
	pfd.cAuxBuffers = 0;
	pfd.iLayerType = PFD_MAIN_PLANE;

	pfd.bReserved = 0;
	pfd.dwLayerMask = 0; pfd.dwVisibleMask = 0; pfd.dwDamageMask = 0;
}

struct OpenGLDrawData
{
	HWND window;
	HDC oglDC;
	HGLRC oglContext;
	LONG width;
	LONG height;
};

bool InitOpenGLContext(OpenGLDrawData &oglData)
{
	PIXELFORMATDESCRIPTOR pfd;
	FillPixelFormatDesc(pfd);

	oglData.oglDC = GetDC(oglData.window);
	if(oglData.oglDC == NULL)
	{
		std::cout << "Cannot get DC !" << std::endl;

		return false;
	}

	int formatIndex = ChoosePixelFormat(oglData.oglDC, &pfd);
	if(formatIndex == 0)
	{
		std::cout << "Cannot choose pixel format !" << std::endl;

		return false;
	}

	PIXELFORMATDESCRIPTOR truePfd;
	DescribePixelFormat(oglData.oglDC, formatIndex, sizeof(PIXELFORMATDESCRIPTOR), &truePfd);

	BOOL ret = SetPixelFormat(oglData.oglDC, formatIndex, &pfd);
	if(ret == FALSE)
	{
		std::cout << "Cannot set pixel format !" << std::endl;

		return false;
	}

	oglData.oglContext = wglCreateContext(oglData.oglDC);
	if(oglData.oglContext == NULL)
	{
		std::cout << "Cannot create context !" << std::endl;

		return false;
	}

	ret = wglMakeCurrent(oglData.oglDC, oglData.oglContext);
	if(ret == FALSE)
	{
		std::cout << "Cannot make current context !" << std::endl;

		return false;
	}

	return true;
}

void InitOpenGL(OpenGLDrawData &oglData)
{
	// get window dimensions
	RECT clientRect;
	GetClientRect(oglData.window, &clientRect);
	LONG width = clientRect.right - clientRect.left;
	LONG height = clientRect.bottom - clientRect.top;
	oglData.width = width;
	oglData.height = height;

	// setup projection matrix
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width, 0, height, -1, 1);
	//gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);

	//glDisable(GL_DEPTH_TEST);
	//glDisable(GL_LIGHTING);
}

void FreeOpenGLContext(OpenGLDrawData &oglData)
{
	wglMakeCurrent(oglData.oglDC, NULL);
	wglDeleteContext(oglData.oglContext);
	ReleaseDC(oglData.window, oglData.oglDC);
}

void DrawFrame(OpenGLDrawData &oglData)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

#if 1
	//glScalef(oglData.width, oglData.height, 1.0f);
	glTranslatef(oglData.width / 2.0f, oglData.height / 2.0f, 0.0f);
	glScalef(oglData.width, oglData.height, 0.0f);

	glBegin(GL_QUADS);
		glColor3ub(255, 0, 0);
		glVertex2f(-0.5f, -0.5f);

		glColor3ub(0, 255, 0);
		glVertex2f(-0.5f, 0.5f);

		glColor3ub(0, 0, 255);
		glVertex2f(0.5f, 0.5f);

		glColor3ub(255, 255, 255);
		glVertex2f(0.5f, -0.5f);
	glEnd();
#endif
#if 0
	glTranslatef(-1.5f,0.0f,-6.0f);						// Move Left 1.5 Units And Into The Screen 6.0
	glBegin(GL_TRIANGLES);								// Drawing Using Triangles
		glColor3f(1.0f,0.0f,0.0f);						// Set The Color To Red
		glVertex3f( 0.0f, 1.0f, 0.0f);					// Top
		glColor3f(0.0f,1.0f,0.0f);						// Set The Color To Green
		glVertex3f(-1.0f,-1.0f, 0.0f);					// Bottom Left
		glColor3f(0.0f,0.0f,1.0f);						// Set The Color To Blue
		glVertex3f( 1.0f,-1.0f, 0.0f);					// Bottom Right
	glEnd();											// Finished Drawing The Triangle
	glTranslatef(3.0f,0.0f,0.0f);						// Move Right 3 Units
	glColor3f(0.5f,0.5f,1.0f);							// Set The Color To Blue One Time Only
	glBegin(GL_QUADS);									// Draw A Quad
		glVertex3f(-1.0f, 1.0f, 0.0f);					// Top Left
		glVertex3f( 1.0f, 1.0f, 0.0f);					// Top Right
		glVertex3f( 1.0f,-1.0f, 0.0f);					// Bottom Right
		glVertex3f(-1.0f,-1.0f, 0.0f);					// Bottom Left
	glEnd();											// Done Drawing The Quad
#endif
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
#if 0
		case WM_PAINT:
		{
			break;
		}
		case WM_ERASEBKGND:
		{
			return 1;
		}
#endif
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
		case WM_KEYDOWN:
		{
			if(wParam == VK_ESCAPE)
			{
				SendMessage(hwnd, WM_CLOSE, 0, 0);
			}

			break;
		}
		// allows drag by grabbing any area of the window
		case WM_NCHITTEST:
		{
			LRESULT hit = DefWindowProc(hwnd, msg, wParam, lParam);
			if (hit == HTCLIENT)
				hit = HTCAPTION;

			return hit;
		}
        default:
		{
            return DefWindowProc(hwnd, msg, wParam, lParam);
		}
    }

    return 0;
}

#define WINDOW_CLASS_NAME "OpenGLWindow"

bool RegisterWindowClass(HINSTANCE instance)
{
    WNDCLASSEX wndClassEx;

	ZERO_STRUCT_WIN32(wndClassEx);
    wndClassEx.lpfnWndProc = WndProc;
    wndClassEx.hInstance = instance;
    wndClassEx.lpszClassName = WINDOW_CLASS_NAME;
	wndClassEx.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClassEx.lpszMenuName = NULL;
	//wndClassEx.hbrBackground = NULL;
	wndClassEx.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wndClassEx.cbWndExtra = sizeof(LONG_PTR);
	wndClassEx.style = CS_HREDRAW | CS_VREDRAW;

	return RegisterClassEx(&wndClassEx) != 0;
}

HWND CreateOpenGLWindow(HINSTANCE instance, int width, int height)
{
	DWORD windowStyle = WS_OVERLAPPEDWINDOW;
	//DWORD windowStyle = WS_POPUP;

	RECT realRect;
	realRect.left = 0;
	realRect.right = width;
	realRect.top = 0;
	realRect.bottom = height;
	AdjustWindowRectEx(&realRect, windowStyle, NULL, 0);

	HWND window = CreateWindow(WINDOW_CLASS_NAME,
		"OpenGL Window",
		windowStyle,
		CW_USEDEFAULT, CW_USEDEFAULT,
		realRect.right - realRect.left,
		realRect.bottom - realRect.top,
		NULL, NULL,
		instance, NULL);

	if(window == NULL)
		return NULL;

	//MakeWindowAeroGlassBackground(window);
	MakeWindowFullyTransparent(window);
	//ExtendAeroBorders(window);

	return window;
}

int MainLoop(OpenGLDrawData &oglData)
{
	MSG msg;
	while(true)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) == TRUE)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if(msg.message == WM_QUIT)
			{
				break;
			}
		}

		DrawFrame(oglData);
		SwapBuffers(oglData.oglDC);
		Sleep(4);
	}

	return msg.wParam;
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	RegisterWindowClass(hInstance);
	HWND window = CreateOpenGLWindow(hInstance, 1024, 768);

	OpenGLDrawData oglData;
	oglData.window = window;
	if(!InitOpenGLContext(oglData))
	{
		return 0;
	}
	InitOpenGL(oglData);

	ShowWindow(window, nCmdShow);
	UpdateWindow(window);
	
	int returnCode = MainLoop(oglData);

	FreeOpenGLContext(oglData);

	return returnCode;
}

