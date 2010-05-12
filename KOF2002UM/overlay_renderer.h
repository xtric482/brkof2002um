#include <atlbase.h>	// ATL CComPtr
#include <ddraw.h>
#pragma once

class overlay_renderer_t{
	void InitializeSurfaces();
	void CreateOverlaySurface();
	void CreatePrimarySurface();
	bool IsInitialized();
	void CleanUp();
	void DisableDesktopWindowManager();
	BOOL CopyBitmapToYUVSurface(const char* fraze);
	CComPtr<IDirectDraw7> m_direct_draw;
	DDCAPS m_caps;

	CComPtr<IDirectDrawSurface7> m_primary_surface, m_overlay_surface;
	IDirectDrawSurface7* m_overlay_back_surface;	// NOT a COM pointer, because we don't control (release) it
	
	bool m_surfaces_lost;
	int OVERLAY_WIDTH, OVERLAY_HEIGHT;
	int OVERLAY_X, OVERLAY_Y;
	DWORD WIN_VERSAO;
public:
	overlay_renderer_t();
	void Ajusta(int X, int Y, int W, int H);
	void Reset();
	void ShowOverlay();
	void update(const char* fraze);
	unsigned long get_versao(void);	
};
