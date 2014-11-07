#ifndef H_MEMORYDC
#define H_MEMORYDC

#include <Windows.h>

class MemoryDC
{
public:
	MemoryDC(int width, int height, bool useAlpha = true);
	~MemoryDC();
	int GetWidth() const;
	int GetHeight() const;
	HDC GetDC() const;
	HBITMAP GetBitmap() const;
	void *GetPixels() const;
private:
	int m_width;
	int m_height;
	bool m_useAlpha;
	HDC m_memoryDC;
	void *m_pixels;
	HBITMAP m_bitmap;
	HBITMAP m_oldBitmap;
};

#endif

