#include <Windows.h>
#include <gdiplus.h>

#include <utils.h>
#include "..\TestOGLInjectorDLL\memorydc.h"

HWND GetTargetWindow()
{
#if 0
	HWND desktop = GetDesktopWindow();
	HWND testWindow = FindWindowEx(desktop, NULL, "InjectionTestWindow", "Injection Test Window");

	return testWindow;
#endif
#if 0
	HWND worker;
	HWND defView;
	HWND listView;

	if(!util::FindDesktopWindows(worker, defView, listView))
	{
		return NULL;
	}

	return listView;
#endif
	//return GetShellWindow();
#if 0
	HWND desktop = GetDesktopWindow();
	HWND prevWindow = NULL;
	HWND currentWindow = NULL;
	
	while(true)
	{
		//currentWindow = FindWindowEx(desktop, prevWindow, "MozillaWindowClass", NULL);
		//currentWindow = FindWindowEx(desktop, prevWindow, "CabinetWClass", "DumpWindow");
		currentWindow = FindWindowEx(desktop, prevWindow, "QWidget", "Playlist");
		if(currentWindow == NULL)
		{
			break;
		}

		LONG windowStyle = GetWindowLong(currentWindow, GWL_STYLE);
		if((windowStyle & WS_VISIBLE) == 0)
		{
			prevWindow = currentWindow;
		}
		else
		{
			break;
		}
	}

	return currentWindow;
#endif
#if 1
	HWND desktop = GetDesktopWindow();
	HWND prevWindow = NULL;
	HWND currentWindow = NULL;
	
	while(true)
	{
		//currentWindow = FindWindowEx(desktop, prevWindow, "MozillaWindowClass", NULL);
		//currentWindow = FindWindowEx(desktop, prevWindow, "CabinetWClass", "DumpWindow");
		currentWindow = FindWindowEx(desktop, prevWindow, "WorkerW", "");
		if(currentWindow == NULL)
		{
			break;
		}

		LONG windowStyle = GetWindowLong(currentWindow, GWL_STYLE);
		if((windowStyle & WS_VISIBLE) != 0 && FindWindowEx(currentWindow, NULL, NULL, NULL) == NULL)
		{
			break;
		}
		else
		{
			prevWindow = currentWindow;
		}
	}

	return currentWindow;
#endif
}

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


int main(int argc, char *argv[])
{
#if 1
	HWND targetWindow = GetTargetWindow();

	RECT windowRect;
	GetWindowRect(targetWindow, &windowRect);

	LONG width = windowRect.right - windowRect.left;
	LONG height = windowRect.bottom - windowRect.top;

	MemoryDC memoryDC(width, height, false);

	HDC windowDC = GetWindowDC(targetWindow);
	int wx = windowRect.left < 0 ? -windowRect.left : 0;
	int wy = windowRect.top < 0 ? -windowRect.top : 0;
	BitBlt(memoryDC.GetDC(), 0, 0, width, height, windowDC, 0, 0, SRCCOPY);
	ReleaseDC(targetWindow, windowDC);

	GdiPlusContext gdiPlusCtx;
	Gdiplus::Bitmap gpBitmap(width, height, PixelFormat24bppRGB);
	Gdiplus::Rect gpRect(0, 0, gpBitmap.GetWidth(), gpBitmap.GetHeight());
	Gdiplus::BitmapData gpBitmapData;
	gpBitmap.LockBits(&gpRect, Gdiplus::ImageLockModeRead, gpBitmap.GetPixelFormat(), &gpBitmapData);
	//memcpy(gpBitmapData.Scan0, bits, gpBitmapData.Stride * gpBitmapData.Height);
	int bmpStride = ((((width * 24) + 31) & ~31) >> 3);
	for(int y = 0; y < height; ++y)
	{
		memcpy(((unsigned char *)gpBitmapData.Scan0) + gpBitmapData.Stride * y, ((unsigned char *)memoryDC.GetPixels()) + bmpStride * (height - y - 1), width * 3);
	}
	//memcpy(gpBitmapData.Scan0, memoryDC.GetPixels(), width * height * 4);
	gpBitmap.UnlockBits(&gpBitmapData);

	CLSID pngClsid;
	GetEncoderClsid(L"image/png", &pngClsid);
	gpBitmap.Save(L"dump.png", &pngClsid);
#endif
#if 0
	HWND window = FindWindowEx(GetDesktopWindow(), NULL, "Progman", NULL);
	if(window != NULL)
	{
		SendMessageTimeout(window, 0x52c, 0, 0, 0, 500, NULL);
	}
#endif
	return 0;
}

