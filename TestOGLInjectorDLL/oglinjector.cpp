#include <Windows.h>
#include <GL/GL.h>

#include <iostream>
#include <sstream>
#include <memory>
#include <vector>
#include <ios>
#include <iomanip>

#define INJECTORCRTSTUB_DLLNAME "TestOGLInjectorDLL.dll"
#include <injectorcrtstub.h>
#include <utils_win32.h>

#include "memorydc.h"
//#include "..\TestOGLWindow\dwmtricks.h"


HWND GetTargetWindow()
{
#if 0
	HWND desktop = GetDesktopWindow();
	HWND testWindow = FindWindowEx(desktop, NULL, "InjectionTestWindow", "Injection Test Window");

	return testWindow;
#endif
#if 1
	HWND worker;
	HWND defView;
	HWND listView;

	if(!util::FindDesktopWindows(worker, defView, listView))
	{
		return NULL;
	}

	return listView;
#endif
}

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
	MemoryDC *memoryDC;
	GLuint iconsTexture;
};

bool InitOpenGLContext(OpenGLDrawData &oglData)
{
	PIXELFORMATDESCRIPTOR pfd;
	FillPixelFormatDesc(pfd);

	oglData.oglDC = GetDC(oglData.window);
	if(oglData.oglDC == NULL)
	{
		util::dcout << "Cannot get DC !" << std::endl;

		return false;
	}

	int formatIndex = ChoosePixelFormat(oglData.oglDC, &pfd);
	if(formatIndex == 0)
	{
		util::dcout << "Cannot choose pixel format !" << std::endl;

		return false;
	}

	PIXELFORMATDESCRIPTOR truePfd;
	DescribePixelFormat(oglData.oglDC, formatIndex, sizeof(PIXELFORMATDESCRIPTOR), &truePfd);

	BOOL ret = SetPixelFormat(oglData.oglDC, formatIndex, &pfd);
	if(ret == FALSE)
	{
		util::dcout << "Cannot set pixel format !" << std::endl;

		return false;
	}

	oglData.oglContext = wglCreateContext(oglData.oglDC);
	if(oglData.oglContext == NULL)
	{
		util::dcout << "Cannot create context !" << std::endl;

		return false;
	}

	ret = wglMakeCurrent(oglData.oglDC, oglData.oglContext);
	if(ret == FALSE)
	{
		util::dcout << "Cannot make current context !" << std::endl;

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
	// HDC is ignored when HGLRC is NULL.
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(oglData.oglContext);
	ReleaseDC(oglData.window, oglData.oglDC);
}

void Init(OpenGLDrawData &oglData)
{
	glGenTextures(1, &oglData.iconsTexture);
	glBindTexture(GL_TEXTURE_2D, oglData.iconsTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, oglData.width, oglData.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
}

void DeInit(OpenGLDrawData &oglData)
{
	glDeleteTextures(1, &oglData.iconsTexture);
}

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	OpenGLDrawData &oglData = *(OpenGLDrawData *)dwData;

	int x = lprcMonitor->left;
	int y = lprcMonitor->top;

	int w = lprcMonitor->right - lprcMonitor->left;
	int h = lprcMonitor->bottom - lprcMonitor->top;

	int iy = oglData.height - h - y;

	glPushMatrix();
	glTranslatef(x, iy, 0.0f);
	glScalef(w, h, 0.0f);

	glBegin(GL_QUADS);
		glColor3ub(255, 0, 0);
		glVertex2f(0.0f, 0.0f);

		glColor3ub(0, 255, 0);
		glVertex2f(0.0f, 1.0f);

		glColor3ub(0, 0, 255);
		glVertex2f(1.0f, 1.0f);

		glColor3ub(255, 255, 255);
		glVertex2f(1.0f, 0.0f);
	glEnd();
	glPopMatrix();

	return TRUE;
}


void DrawFrame(OpenGLDrawData &oglData)
{
	//OutputDebugString("Drawing frame ...\n");

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	EnumDisplayMonitors(oglData.oglDC, NULL, MonitorEnumProc, (LPARAM)&oglData);

#if 0
	//glScalef(oglData.width, oglData.height, 1.0f);
	glPushMatrix();
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
	glPopMatrix();
#endif
#if 1
	glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // non-premultiplied alpha
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // premultiplied alpha
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, oglData.iconsTexture);
	glColor3ub(255, 255, 255);
	glScalef(oglData.width, oglData.height, 0.0f);

	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(0.0f, 0.0f);

		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(0.0f, 1.0f);

		glTexCoord2f(1.0f, 1.0f);
		glVertex2f(1.0f, 1.0f);

		glTexCoord2f(1.0f, 0.0f);
		glVertex2f(1.0f, 0.0f);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
#endif

	//OutputDebugString("Drew frame.\n");
}

void DrawLoop(OpenGLDrawData &oglData)
{
	util::dcout << "Starting main loop ..." << std::endl;
	
	LARGE_INTEGER frequency;
	LARGE_INTEGER lastTime;
	LARGE_INTEGER currentTime;

	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&lastTime);

	double totalElapsedTime = 0.0;
	bool running = true;
	while(running)
	{
		DrawFrame(oglData);
		SwapBuffers(oglData.oglDC);

		QueryPerformanceCounter(&currentTime);
		double deltaTime = (double)(currentTime.QuadPart - lastTime.QuadPart) / (double)frequency.QuadPart;
		totalElapsedTime += deltaTime;
		lastTime = currentTime;

		if(totalElapsedTime >= 10.0)
		{
			running = false;
			util::dcout << "Exiting main loop ..." << std::endl;
			break;
		}

		Sleep(4);
	}
}

WNDPROC g_originalWndProc;
OpenGLDrawData *g_oglData;

LRESULT CALLBACK HookWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
#if 0
		case WM_PAINT:
		{
			ValidateRect(hWnd, NULL);
			return 0;
		}
		case WM_ERASEBKGND:
		{
			ValidateRect(hWnd, NULL);
			return 1;
		}
#endif
#if 1
		case WM_PAINT:
		{
			OpenGLDrawData &oglData = *g_oglData;

			RECT updateRect;
			GetUpdateRect(hWnd, &updateRect, FALSE);

#if 0
			LONG updateX = updateRect.left;
			LONG updateY = updateRect.top;
			LONG updateW = updateRect.right - updateRect.left;
			LONG updateH = updateRect.bottom - updateRect.top;
			{
				std::ostringstream ss;
				ss << "Asked to update rect { X = " << updateX << ", Y = " << updateY << ", W = " << updateW << ", H = " << updateH << " }\n";
				OutputDebugString(ss.str().c_str());
			}
#endif

#if 0
			BOOL ret = wglMakeCurrent(oglData.oglDC, oglData.oglContext);
			{
				std::ostringstream ss;
				ss << "wglMakeCurrent(oglData.oglDC, oglData.oglContext) = " << (ret == TRUE ? "TRUE" : "FALSE" ) << "\n";
				OutputDebugString(ss.str().c_str());
			}
#endif
			wglMakeCurrent(oglData.oglDC, oglData.oglContext);
#if 1
			// SelectClipRgn + CreateRectRgn
			// the window honors the region selected.
			HRGN updateRegion = CreateRectRgnIndirect(&updateRect);
			SelectClipRgn(oglData.memoryDC->GetDC(), updateRegion);
			DeleteObject(updateRegion);
			SendMessage(hWnd, WM_PRINTCLIENT, (WPARAM)oglData.memoryDC->GetDC(), PRF_CLIENT);
			GdiFlush();

#if 0
			LONG updateX = updateRect.left;
			LONG updateY = updateRect.top;
			LONG updateW = updateRect.right - updateRect.left;
			LONG updateH = updateRect.bottom - updateRect.top;

//			if(updateY != 0)
//			{
//				updateY = oglData.height - updateY - 1;
//			}
#if 1
			{
				std::ostringstream ss;
				ss << "Asked to update rect { X = " << updateX << ", Y = " << updateY << ", W = " << updateW << ", H = " << updateH << " }\n";
				OutputDebugString(ss.str().c_str());
			}
#endif
			glBindTexture(GL_TEXTURE_2D, oglData.iconsTexture);
			//int pitch = (oglData.width) * 4;
			//unsigned char *pixelOffset = ((unsigned char *)oglData.memoryDC->GetPixels()) + pitch * updateY + updateX * 4;

			// glPixelStore to offset the reading of pixels
			glPixelStorei(GL_UNPACK_ROW_LENGTH, oglData.width);
			glPixelStorei(GL_UNPACK_SKIP_ROWS, updateY);
			glPixelStorei(GL_UNPACK_SKIP_PIXELS, updateX);
			glTexSubImage2D(GL_TEXTURE_2D, 0, updateX, updateY, updateW, updateH, GL_BGRA_EXT, GL_UNSIGNED_BYTE, oglData.memoryDC->GetPixels());
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
			glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
			glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
#endif
#if 1
#if 0
			{
				std::ostringstream ss;

				GLint paramVal;

				glGetIntegerv(GL_UNPACK_ALIGNMENT, &paramVal);
				ss << "GL_UNPACK_ALIGNMENT = " << paramVal << "\n";

				glGetIntegerv(GL_UNPACK_SKIP_ROWS, &paramVal);
				ss << "GL_UNPACK_SKIP_ROWS = " << paramVal << "\n";

				glGetIntegerv(GL_UNPACK_SKIP_PIXELS, &paramVal);
				ss << "GL_UNPACK_SKIP_PIXELS = " << paramVal << "\n";

				glGetIntegerv(GL_UNPACK_ROW_LENGTH, &paramVal);
				ss << "GL_UNPACK_ROW_LENGTH = " << paramVal << "\n";

				OutputDebugString(ss.str().c_str());
			}
#endif
			glBindTexture(GL_TEXTURE_2D, oglData.iconsTexture);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, oglData.width, oglData.height, GL_BGRA_EXT, GL_UNSIGNED_BYTE, oglData.memoryDC->GetPixels());
#endif
#endif

			DrawFrame(oglData);

			SwapBuffers(oglData.oglDC);
			wglMakeCurrent(NULL, NULL);

			ValidateRect(hWnd, &updateRect);

			return 0;
		}
#endif
	}

	return CallWindowProc(g_originalWndProc, hWnd, msg, wParam, lParam);
}

void CheckWindow(HWND window)
{
	std::vector<char> buffer(64);

	if(GetClassName(window, buffer.data(), buffer.size()) == 0)
		return;

	HINSTANCE windowHInstance = (HINSTANCE)GetWindowLongPtr(window, GWLP_HINSTANCE);

	WNDCLASSEX classInfo;
	GetClassInfoEx(windowHInstance, buffer.data(), &classInfo);

	LONG_PTR windowUserData = GetWindowLongPtr(window, GWLP_USERDATA);

	util::dcout << "Extra bytes: " << classInfo.cbWndExtra << " (" << (classInfo.cbWndExtra / sizeof(LONG_PTR)) << " pointers of " << sizeof(LONG_PTR) << " bytes)" << std::endl;

	std::ios::fmtflags f(util::dcout.flags());
	util::dcout << "Window user data: 0x" << std::setfill('0') << std::setw(sizeof(LONG_PTR) * 2) << std::hex << windowUserData << std::endl;
	util::dcout.flags(f);
}

void InjectorProc(void *injectorModule, unsigned int dataSize, const void *data)
{
	util::dcout << "In inject proc." << std::endl;

#if 0
	util::dcout << "Waiting for debugger ..." << std::endl;

	if(IsDebuggerPresent() == FALSE)
	{
		while(IsDebuggerPresent() == FALSE)
		{
			Sleep(100);
		}

		DebugBreak();
	}

	util::dcout << "Debugger attached." << std::endl;
#endif

	HWND targetWindow = GetTargetWindow();
	if(targetWindow == NULL)
	{
		util::dcout << "Cannot find window !" << std::endl;

		return;
	}

	CheckWindow(targetWindow);

	OpenGLDrawData oglData;
	oglData.window = targetWindow;
	if(!InitOpenGLContext(oglData))
	{
		return;
	}
	InitOpenGL(oglData);

	MemoryDC memoryDC(oglData.width, oglData.height);
	oglData.memoryDC = &memoryDC;
	Init(oglData);

	wglMakeCurrent(NULL, NULL);
	g_oglData = &oglData;

	//MakeWindowFullyTransparent(targetWindow);
	WNDPROC originalWndProc = (WNDPROC)GetWindowLongPtr(targetWindow, GWLP_WNDPROC);
	g_originalWndProc = originalWndProc;
	SetWindowLongPtr(targetWindow, GWLP_WNDPROC, (LONG_PTR)HookWndProc);
	InvalidateRect(targetWindow, NULL, TRUE);

	//DrawLoop(oglData);
	//DrawFrame(oglData);
	Sleep(30000);

	wglMakeCurrent(oglData.oglDC, oglData.oglContext);
	DeInit(oglData);
	FreeOpenGLContext(oglData);

	SetWindowLongPtr(targetWindow, GWLP_WNDPROC, (LONG_PTR)originalWndProc);
	//MakeWindowUndo(targetWindow);
	InvalidateRect(targetWindow, NULL, TRUE);

	util::dcout << "Exiting inject proc." << std::endl;
}

