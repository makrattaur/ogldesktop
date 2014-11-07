#include "dwmtricks.h"

#include <dwmapi.h>

bool MakeWindowAeroGlassBackground(HWND hwnd)
{
	DWM_BLURBEHIND bb;

	bb.dwFlags = DWM_BB_ENABLE;
	bb.fEnable = TRUE;
	bb.hRgnBlur = NULL;
	//bb.fTransitionOnMaximized = FALSE; // Bit DWM_BB_TRANSITIONONMAXIMIZED not specified.

	return DwmEnableBlurBehindWindow(hwnd, &bb) == S_OK;
}

bool MakeWindowFullyTransparent(HWND hwnd)
{
	DWM_BLURBEHIND bb;

	HRGN hRgn = CreateRectRgn(0, 0, -1, -1);
	bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
	bb.fEnable = TRUE;
	bb.hRgnBlur = hRgn;
	//bb.fTransitionOnMaximized = FALSE; // Bit DWM_BB_TRANSITIONONMAXIMIZED not specified.

	HRESULT ret = DwmEnableBlurBehindWindow(hwnd, &bb);

	DeleteObject(hRgn);

	return ret == S_OK;
}

bool MakeWindowUndo(HWND hwnd)
{
	DWM_BLURBEHIND bb;

	bb.dwFlags = DWM_BB_ENABLE;
	bb.fEnable = FALSE;
	//bb.fTransitionOnMaximized = FALSE; // Bit DWM_BB_TRANSITIONONMAXIMIZED not specified.

	return DwmEnableBlurBehindWindow(hwnd, &bb) == S_OK;
}

bool ExtendAeroBorders(HWND hwnd)
{
	MARGINS margins;
	margins.cxLeftWidth = -1;
	margins.cxRightWidth = -1;
	margins.cyBottomHeight = -1;
	margins.cyTopHeight = -1;

	return DwmExtendFrameIntoClientArea(hwnd, &margins) == S_OK;
}


