#include <Windows.h>
#include <gdiplus.h>
#include <Psapi.h>

#include <string>
#include <sstream>
#include <iomanip>
#include <vector>

//#define INJECTORCRTSTUB_DLLNAME "DrawHookInjectee.dll"
#include <injectorcrtstub.h>
#include <utils_win32.h>


bool IsExplorer();


int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
   UINT  num = 0;          // number of image encoders
   UINT  size = 0;         // size of the image encoder array in bytes

   Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

   Gdiplus::GetImageEncodersSize(&num, &size);
   if(size == 0)
      return -1;  // Failure

   pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
   if(pImageCodecInfo == NULL)
      return -1;  // Failure

   GetImageEncoders(num, size, pImageCodecInfo);

   for(UINT j = 0; j < num; ++j)
   {
      if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
      {
         *pClsid = pImageCodecInfo[j].Clsid;
         free(pImageCodecInfo);
         return j;  // Success
      }    
   }

   free(pImageCodecInfo);
   return -1;  // Failure
}


void DumpDesktopIcons()
{
	OutputDebugString("[DHI] DumpDesktopIcons() entry.\n");

	HWND worker;
	HWND defView;
	HWND listView;

	if(!util::FindDesktopWindows(worker, defView, listView))
	{
		return;
	}

	OutputDebugString("[DHI] DumpDesktopIcons() has windows.\n");

	HDC memoryDC;
	RECT listViewRect;
	void *bits;

	GetClientRect(listView, &listViewRect);

	memoryDC = CreateCompatibleDC(NULL);
	BITMAPINFO bi;
	ZeroMemory(&bi, sizeof(bi));
	bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
	bi.bmiHeader.biWidth = listViewRect.right - listViewRect.left;
	bi.bmiHeader.biHeight = listViewRect.bottom - listViewRect.top;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;
	HBITMAP bitmap = CreateDIBSection(memoryDC, &bi, DIB_RGB_COLORS, &bits, NULL, 0);
	SelectObject(memoryDC, bitmap);

	OutputDebugString("[DHI] DumpDesktopIcons() created memory DC.\n");

	//InvalidateRect(defView, NULL, TRUE);
	SendMessage(listView, WM_PRINTCLIENT, (WPARAM)memoryDC, PRF_CLIENT);
	//SendMessage(listView, WM_PAINT, (WPARAM)memoryDC, 0);
	GdiFlush();

	OutputDebugString("[DHI] DumpDesktopIcons() sent WM_PRINTCLIENT.\n");

	Gdiplus::Bitmap gpBitmap(bi.bmiHeader.biWidth, bi.bmiHeader.biHeight, PixelFormat32bppPARGB);
	Gdiplus::Rect gpRect(0, 0, gpBitmap.GetWidth(), gpBitmap.GetHeight());
	Gdiplus::BitmapData gpBitmapData;
	gpBitmap.LockBits(&gpRect, Gdiplus::ImageLockModeRead, gpBitmap.GetPixelFormat(), &gpBitmapData);
	//memcpy(gpBitmapData.Scan0, bits, gpBitmapData.Stride * gpBitmapData.Height);
	int pitch = (listViewRect.right - listViewRect.left) * 4;
	for(int y = 0; y < listViewRect.bottom - listViewRect.top; ++y)
	{
		memcpy(((unsigned char *)gpBitmapData.Scan0) + gpBitmapData.Stride * y, ((unsigned char *)bits) + pitch * (listViewRect.bottom - listViewRect.top - y - 1), pitch);
	}
	gpBitmap.UnlockBits(&gpBitmapData);

	OutputDebugString("[DHI] DumpDesktopIcons() copied pixels to GDI+ bitmap.\n");

	CLSID pngClsid;
	GetEncoderClsid(L"image/png", &pngClsid);
	gpBitmap.Save(L"C:\\Users\\Michael\\Desktop\\pr.png", &pngClsid);

	OutputDebugString("[DHI] DumpDesktopIcons() saved bitmap.\n");

	DeleteObject(bitmap);
	DeleteObject(memoryDC);

	OutputDebugString("[DHI] DumpDesktopIcons() done.\n");
}

struct WindowHookData
{
	void *bits;
	HDC memoryDC;
	RECT windowRect;
	LONG width;
	LONG height;
	BITMAPINFO bi;
	HBITMAP bitmap;
	BLENDFUNCTION alphaBlendFunction;

	HBITMAP backgroundBitmap;
	HDC backgroundDC;

	HBITMAP compositeBitmap;
	HDC compositeDC;

	HPEN pen;
	HBRUSH clearBrush;
};

WNDPROC g_originalWndProc;
WindowHookData *g_whd;

LRESULT CALLBACK HookWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_PAINT:
		{
			WindowHookData &whd = *g_whd;

#if 0
			RECT invalidRect;
			GetUpdateRect(hWnd, &invalidRect, FALSE);
			CallWindowProc(g_originalWndProc, hWnd, msg, wParam, lParam);
			InvalidateRect(hWnd, &invalidRect, FALSE);
#endif

			PAINTSTRUCT ps;
			HDC dc = BeginPaint(hWnd, &ps);

			//RECT clientRect;
			//GetClientRect(hWnd, &clientRect);
			//FillRect(dc, &clientRect, whd.clearBrush);
			//BitBlt(dc, 0, 0, whd.width, whd.height, whd.backgroundDC, 0, 0, SRCCOPY);

#if 0
			LONG updateX = ps.rcPaint.left;
			LONG updateY = ps.rcPaint.top;
			LONG updateW = ps.rcPaint.right - ps.rcPaint.left;
			LONG updateH = ps.rcPaint.bottom - ps.rcPaint.top;

			std::ostringstream ss;
			ss << "Asked to update rect { X = " << updateX << ", Y = " << updateY << ", W = " << updateW << ", H = " << updateH << " }\n";
			OutputDebugString(ss.str().c_str());

			RECT clientRect;
			GetClientRect(hWnd, &clientRect);
			HPEN oldPen = (HPEN)SelectObject(dc, whd.pen);
			
			MoveToEx(dc, clientRect.left, clientRect.top, NULL);
			LineTo(dc, clientRect.right, clientRect.bottom);

			MoveToEx(dc, clientRect.right, clientRect.top, NULL);
			LineTo(dc, clientRect.left, clientRect.bottom);

			SelectObject(dc, oldPen);
#endif
#if 0
			FillRect(whd.memoryDC, &whd.windowRect, whd.clearBrush);
			CallWindowProc(g_originalWndProc, hWnd, WM_PRINTCLIENT, (WPARAM)whd.memoryDC, PRF_CLIENT);
			AlphaBlend(dc, 0, 0, whd.width, whd.height, whd.memoryDC, 0, 0, whd.width, whd.height, whd.alphaBlendFunction);
#endif
#if 0
			BitBlt(whd.compositeDC, 0, 0, whd.width, whd.height, whd.backgroundDC, 0, 0, SRCCOPY);
			CallWindowProc(g_originalWndProc, hWnd, WM_PRINTCLIENT, (WPARAM)whd.memoryDC, PRF_CLIENT);
			AlphaBlend(whd.compositeDC, 0, 0, whd.width, whd.height, whd.memoryDC, 0, 0, whd.width, whd.height, whd.alphaBlendFunction);
			BitBlt(dc, 0, 0, whd.width, whd.height, whd.compositeDC, 0, 0, SRCCOPY);
#endif
#if 1
			LONG updateX = ps.rcPaint.left;
			LONG updateY = ps.rcPaint.top;
			LONG updateW = ps.rcPaint.right - ps.rcPaint.left;
			LONG updateH = ps.rcPaint.bottom - ps.rcPaint.top;

			BitBlt(whd.compositeDC, updateX, updateY, updateW, updateH, whd.backgroundDC, updateX, updateY, SRCCOPY);
			CallWindowProc(g_originalWndProc, hWnd, WM_PRINTCLIENT, (WPARAM)whd.memoryDC, PRF_CLIENT);
			AlphaBlend(whd.compositeDC, updateX, updateY, updateW, updateH, whd.memoryDC, updateX, updateY, updateW, updateH, whd.alphaBlendFunction);
			BitBlt(dc, updateX, updateY, updateW, updateH, whd.compositeDC, updateX, updateY, SRCCOPY);
#endif
#if 0
			LONG updateX = ps.rcPaint.left;
			LONG updateY = ps.rcPaint.top;
			LONG updateW = ps.rcPaint.right - ps.rcPaint.left;
			LONG updateH = ps.rcPaint.bottom - ps.rcPaint.top;

#if 0
			std::ostringstream ss;
			ss << "Asked to update rect { X = " << updateX << ", Y = " << updateY << ", W = " << updateW << ", H = " << updateH << " }\n";
			OutputDebugString(ss.str().c_str());
#endif

			FillRect(whd.compositeDC, &ps.rcPaint, whd.clearBrush);
			//CallWindowProc(g_originalWndProc, hWnd, WM_PRINTCLIENT, (WPARAM)whd.memoryDC, PRF_CLIENT);
			CallWindowProc(g_originalWndProc, hWnd, WM_PAINT, (WPARAM)whd.memoryDC, 0);
			AlphaBlend(whd.compositeDC, updateX, updateY, updateW, updateH, whd.memoryDC, updateX, updateY, updateW, updateH, whd.alphaBlendFunction);
			BitBlt(dc, updateX, updateY, updateW, updateH, whd.compositeDC, updateX, updateY, SRCCOPY);
#endif
			EndPaint(hWnd, &ps);

			return 0;
		}
	}

	return CallWindowProc(g_originalWndProc, hWnd, msg, wParam, lParam);
}

void InitWindowHooking(WindowHookData &whd, HWND window)
{
#if 1
	GetClientRect(window, &whd.windowRect);
	whd.width = whd.windowRect.right - whd.windowRect.left;
	whd.height = whd.windowRect.bottom - whd.windowRect.top;

	// create DC for list view contents
	whd.memoryDC = CreateCompatibleDC(NULL);
	ZeroMemory(&whd.bi, sizeof(whd.bi));
	whd.bi.bmiHeader.biSize = sizeof(whd.bi.bmiHeader);
	whd.bi.bmiHeader.biWidth = whd.width;
	whd.bi.bmiHeader.biHeight = whd.height;
	whd.bi.bmiHeader.biPlanes = 1;
	whd.bi.bmiHeader.biBitCount = 32;
	whd.bi.bmiHeader.biCompression = BI_RGB;
	whd.bitmap = CreateDIBSection(whd.memoryDC, &whd.bi, DIB_RGB_COLORS, &whd.bits, NULL, 0);
	SelectObject(whd.memoryDC, whd.bitmap);

	whd.alphaBlendFunction.AlphaFormat = AC_SRC_ALPHA;
	whd.alphaBlendFunction.BlendFlags = 0;
	whd.alphaBlendFunction.BlendOp = AC_SRC_OVER;
	whd.alphaBlendFunction.SourceConstantAlpha = 255;

	// create DC for composited contents
	whd.compositeDC = CreateCompatibleDC(NULL);
	BITMAPINFO biComposite;
	ZeroMemory(&biComposite, sizeof(biComposite));
	biComposite.bmiHeader.biSize = sizeof(biComposite.bmiHeader);
	biComposite.bmiHeader.biWidth = whd.width;
	biComposite.bmiHeader.biHeight = whd.height;
	biComposite.bmiHeader.biPlanes = 1;
	biComposite.bmiHeader.biBitCount = 32;
	biComposite.bmiHeader.biCompression = BI_RGB;
	void *compositeBits;
	whd.compositeBitmap = CreateDIBSection(whd.compositeDC, &biComposite, DIB_RGB_COLORS, &compositeBits, NULL, 0);
	SelectObject(whd.compositeDC, whd.compositeBitmap);

	// create background DC and bitmap
	whd.backgroundDC = CreateCompatibleDC(NULL);
	BITMAPINFO bi;
	ZeroMemory(&bi, sizeof(bi));
	bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
	bi.bmiHeader.biWidth = whd.width;
	bi.bmiHeader.biHeight = whd.height;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;
	void *backgroundBits;
	whd.backgroundBitmap = CreateDIBSection(whd.backgroundDC, &bi, DIB_RGB_COLORS, &backgroundBits, NULL, 0);
	SelectObject(whd.backgroundDC, whd.backgroundBitmap);

	// tile image across background DC
	Gdiplus::Bitmap image(L"C:\\Users\\Michael\\Desktop\\test-tile.png");
	HBITMAP imageBitmap;
	image.GetHBITMAP(Gdiplus::Color(0, 0, 0), &imageBitmap);
	HDC imageDC = CreateCompatibleDC(NULL);
	SelectObject(imageDC, imageBitmap);

	int w = image.GetWidth();
	int h = image.GetHeight();
	int countX = (int)ceil(whd.width / (float)w);
	int countY = (int)ceil(whd.height / (float)h);

	for(int y = 0; y < countY; y++)
	{
		for(int x = 0; x < countX; x++)
		{
			BitBlt(whd.backgroundDC, x * w, y * h, w, h, imageDC, 0, 0, SRCCOPY);
		}
	}

	DeleteObject(imageDC);
	DeleteObject(imageBitmap);

#endif

	// create drawing objects
	whd.pen = CreatePen(PS_SOLID, 2, RGB(0, 255, 0));
	whd.clearBrush = CreateSolidBrush(0xff000000);

	g_whd = &whd;
}

void DeInitWindowHooking(WindowHookData &whd)
{
	DeleteObject(whd.pen);
	DeleteObject(whd.clearBrush);
#if 1
	DeleteObject(whd.memoryDC);
	DeleteObject(whd.bitmap);

	DeleteObject(whd.backgroundDC);
	DeleteObject(whd.backgroundBitmap);

	DeleteObject(whd.compositeDC);
	DeleteObject(whd.compositeBitmap);
#endif
}

void HookWindow()
{
	HWND worker;
	HWND defView;
	HWND listView;

	if(!util::FindDesktopWindows(worker, defView, listView))
	{
		return;
	}

	std::ostringstream ss;
	ss << std::hex << std::uppercase << std::setfill('0') <<
		"worker(0x" << std::setw(8) << (uintptr_t)worker << "), " <<
		"defView(0x" << std::setw(8) << (uintptr_t)defView << "), " <<
		"listView(0x" << std::setw(8) << (uintptr_t)listView << ")\n";
	OutputDebugString(ss.str().c_str());

	HWND &targetWindow = listView;

	WNDPROC originalWndProc = (WNDPROC)GetWindowLongPtr(targetWindow, GWLP_WNDPROC);
	g_originalWndProc = originalWndProc;

	WindowHookData whd;
	InitWindowHooking(whd, targetWindow);

	SetWindowLongPtr(targetWindow, GWLP_WNDPROC, (LONG_PTR)HookWndProc);
	InvalidateRect(targetWindow, NULL, TRUE);

	Sleep(20000);

	SetWindowLongPtr(targetWindow, GWLP_WNDPROC, (LONG_PTR)originalWndProc);
	InvalidateRect(targetWindow, NULL, TRUE);

	DeInitWindowHooking(whd);
}

class GdiPlusContext
{
public:
	GdiPlusContext()
	{
		Gdiplus::GdiplusStartupInput gdiPlusStartupInput;
		gdiPlusStartupInput.GdiplusVersion = 1;
		gdiPlusStartupInput.DebugEventCallback = NULL;
		gdiPlusStartupInput.SuppressBackgroundThread = FALSE;
		gdiPlusStartupInput.SuppressExternalCodecs = FALSE;

		Gdiplus::GdiplusStartup(&m_token, &gdiPlusStartupInput, NULL);
	}
	~GdiPlusContext()
	{
		Gdiplus::GdiplusShutdown(m_token);
	}
private:
	ULONG_PTR m_token;
};

void InjectorProc(void *injectorModule, unsigned int dataSize, const void *data)
{
	if(!IsExplorer())
	{
		return;
	}

#if 0
	Sleep(5000);

	{
		GdiPlusContext ctx;

		DumpDesktopIcons();
	}
#endif

#if 1
	{
		GdiPlusContext ctx;

		OutputDebugString("Hooking window ...\n");
		HookWindow();
		OutputDebugString("All done !\n");
	}
#endif
}

bool IsExplorer()
{
	HANDLE process = GetCurrentProcess();
	std::vector<char> buffer(1024);
	DWORD count = buffer.size();
	QueryFullProcessImageName(process, 0, &buffer[0], &count);

	std::string currentProcessPath(&buffer[0], count);
	std::string currentProcessImageName = currentProcessPath.substr(currentProcessPath.find_last_of('\\') + 1);

	return currentProcessImageName.compare("explorer.exe") == 0;
}

