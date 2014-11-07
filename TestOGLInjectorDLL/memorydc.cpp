#include "memorydc.h"


MemoryDC::MemoryDC(int width, int height, bool useAlpha) : m_width(width), m_height(height), m_useAlpha(useAlpha)
{
	m_memoryDC = CreateCompatibleDC(NULL);

	BITMAPINFO bi;
	ZeroMemory(&bi, sizeof(bi));
	bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
	bi.bmiHeader.biWidth = width;
	bi.bmiHeader.biHeight = height;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = m_useAlpha ? 32 : 24;
	bi.bmiHeader.biCompression = BI_RGB;

	m_bitmap = CreateDIBSection(m_memoryDC, &bi, DIB_RGB_COLORS, &m_pixels, NULL, 0);
	m_oldBitmap = (HBITMAP)SelectObject(m_memoryDC, m_bitmap);
}

MemoryDC::~MemoryDC()
{
	SelectObject(m_memoryDC, m_oldBitmap);
	DeleteObject(m_bitmap);
	DeleteObject(m_memoryDC);
}

int MemoryDC::GetWidth() const
{
	return m_width;
}

int MemoryDC::GetHeight() const
{
	return m_height;
}

HDC MemoryDC::GetDC() const
{
	return m_memoryDC;
}

HBITMAP MemoryDC::GetBitmap() const
{
	return m_bitmap;
}

void *MemoryDC::GetPixels() const
{
	return m_pixels;
}

