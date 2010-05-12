//#include "StdAfx.h"
#include "overlay_renderer.h"
//#include "exception.h"

//=================================================================
//construtor da classe
//=================================================================
overlay_renderer_t::overlay_renderer_t() : m_surfaces_lost(false), OVERLAY_X(0), OVERLAY_Y(0), OVERLAY_WIDTH(0), OVERLAY_HEIGHT(0), WIN_VERSAO(0)
{
	
	//verifica a versão do windows
	OSVERSIONINFOA osvi;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOA));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
	GetVersionExA(&osvi);
	if(osvi.dwMajorVersion > 5){	//5.1=xp, 6.1==win7		
		DisableDesktopWindowManager();		
	}
	WIN_VERSAO=osvi.dwMajorVersion;
	
	// Create the main DirectDraw object
	if(FAILED(DirectDrawCreateEx(0, (void**)&m_direct_draw, IID_IDirectDraw7, 0))){
		MessageBoxA(NULL,"Erro criando dispositivo","Erro", 0);
	}

	ZeroMemory(&m_caps, sizeof(m_caps));
	m_caps.dwSize = sizeof(m_caps);
	if(FAILED(m_direct_draw->GetCaps(&m_caps, 0))){
		MessageBoxA(NULL,"Erro em GetCaps 1","Erro", 0);
	}
}


//=================================================================
//ajusta o tamanho do overlay inicial
//=================================================================
void overlay_renderer_t::Ajusta(int X, int Y, int W, int H){
	OVERLAY_X=X;
	OVERLAY_Y=Y;
	OVERLAY_WIDTH=W;
	OVERLAY_HEIGHT=H;
}


//=================================================================
//verifica se as superficies foram inicializadas
//=================================================================
bool overlay_renderer_t::IsInitialized(){
	return m_overlay_surface && m_overlay_back_surface;
}


//==========================================================================
//reseta as superficies foram inicializadas
//==========================================================================
void overlay_renderer_t::Reset(){
	if(IsInitialized()){
		CleanUp();
	}

	if(FAILED(m_direct_draw->SetCooperativeLevel(0, DDSCL_NORMAL))){
		MessageBoxA(NULL,"Erro no SetCooperativeLevel", "Erro", 0);
	}

	InitializeSurfaces();

	if(FAILED(m_direct_draw->GetCaps(&m_caps, 0))){
		MessageBoxA(NULL,"Erro em GetCaps 2","Erro", 0);
	}
}


//=================================================================
//cria as duas superficies
//=================================================================
void overlay_renderer_t::InitializeSurfaces(){
	CreatePrimarySurface();
	CreateOverlaySurface();
}


//=================================================================
//cria a superficie primaria
//=================================================================
void overlay_renderer_t::CreatePrimarySurface(){
	DDSURFACEDESC2 ddsd;
	ZeroMemory(&ddsd,sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	
	if(FAILED(m_direct_draw->CreateSurface(&ddsd, &m_primary_surface, 0))){
		MessageBoxA(NULL,"Erro em primary_surface","Erro", 0);
	}
}


//=================================================================
//cria a superficie overlay
//=================================================================
void overlay_renderer_t::CreateOverlaySurface(){
	DDSURFACEDESC2 ddsd;	
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_BACKBUFFERCOUNT | DDSD_PIXELFORMAT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OVERLAY | DDSCAPS_FLIP | DDSCAPS_COMPLEX | DDSCAPS_VIDEOMEMORY;
	ddsd.dwBackBufferCount = 1;
	ddsd.dwWidth = OVERLAY_WIDTH;
	ddsd.dwHeight = OVERLAY_HEIGHT;
	
	//formatos validos 0 4
	DDPIXELFORMAT dd_format[] = {
		// PIXEL_FORMAT_32BPP
		{ sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 32, 0xFF0000, 0x0000FF00, 0x000000FF, 0},
		// PIXEL_FORMAT_16BPP_R5G6B5
		{ sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 16, 0xF800, 0x07E0, 0x001F, 0},
		// PIXEL_FORMAT_16BPP_R5G5B5		
		{ sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 16, 0x7C00, 0x03e0, 0x001F, 0},
		// PIXEL_FORMAT_UYVY
		{ sizeof(DDPIXELFORMAT), DDPF_FOURCC, MAKEFOURCC('U', 'Y', 'V', 'Y'), 0, 0, 0, 0, 0},
		// PIXEL_FORMAT_YUY2
		{ sizeof(DDPIXELFORMAT), DDPF_FOURCC, MAKEFOURCC('Y', 'U', 'Y', '2'), 0, 0, 0, 0, 0}
	};
	
	HRESULT overlayResult = S_OK;

	for(int a=0;a<5;a++){
		ddsd.ddpfPixelFormat = dd_format[a];
		overlayResult = m_direct_draw->CreateSurface(&ddsd, &m_overlay_surface, 0);
		if(SUCCEEDED(overlayResult)){
			break;
		}
	}
	
	if(FAILED(overlayResult)){
		MessageBoxA(NULL,"Erro em m_overlay_surface","Erro", 0);
	}

	DDSCAPS2 ddscaps;
	ZeroMemory(&ddscaps, sizeof(ddscaps));
	ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
	if(FAILED(m_overlay_surface->GetAttachedSurface(&ddscaps, &m_overlay_back_surface))){
		MessageBoxA(NULL,"Erro em m_overlay_back_surface","Erro", 0);
	}
}


//==========================================================
//atualiza o texto e posição do overlay
//==========================================================
void overlay_renderer_t::update(const char* fraze){
	HRESULT hr;
	
	// Check the cooperative level before rendering
	if(FAILED(hr = m_direct_draw->TestCooperativeLevel())){
		switch(hr){
			case DDERR_EXCLUSIVEMODEALREADYSET:
				// Do nothing because some other app has exclusive mode
				m_surfaces_lost = true;				
				break;

			case DDERR_WRONGMODE:
				// The display mode changed on us. Update the surfaces accordingly
				CleanUp();
				Reset();
				ShowOverlay();
				break;

			default:								
				return;			
		}
	}

	if(m_surfaces_lost || FAILED(m_primary_surface->IsLost())){
		if(FAILED(m_direct_draw->RestoreAllSurfaces())){
			try{
				Reset();
			}catch(...){
				return;
			}
		}
		ShowOverlay();
		m_surfaces_lost = false;
	}
	
	//somente se for winxp
	if(WIN_VERSAO <= 5){
		//declara as variaveis usadas
		HDC dc;
		HFONT mfont;
		RECT R;
		HBRUSH cor_ret1, cor_ret2;

		//define as cores dos retangulos
		cor_ret1=CreateSolidBrush(0x000066FF);	//laranja
		cor_ret2=CreateSolidBrush(0x00000000);	//preto
	
		//congela o device context
		m_overlay_back_surface->GetDC(&dc);

		//imprime o primeiro retangulo
		SetRect(&R, 0, 0, OVERLAY_WIDTH, OVERLAY_HEIGHT);
		FillRect(dc, &R, cor_ret1);
	
		//imprime o segundo retangulo, para o primeiro dar uma impressão de ser bordas
		SetRect(&R, 2, 2, OVERLAY_WIDTH-2, OVERLAY_HEIGHT-2);
		FillRect(dc, &R, cor_ret2);

		//cria a fonte a ser usado
		mfont=CreateFontA(OVERLAY_HEIGHT, 0, 0, 0, FW_BOLD, false, false, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, DEFAULT_PITCH, "Arial");
		SelectObject(dc, mfont);

		//atribui a cor da fonte e fundo
		SetTextColor(dc, 0x0000CCFF);	//laranja
		SetBkColor(dc, 0x00000000);		//preto

		//imprime o texto na posicao informada
		DrawText(dc, fraze, strlen(fraze), &R, DT_CENTER);

		//delete a font anterior	
		if(mfont){DeleteObject(SelectObject(dc, mfont));}

		//libera o DC e atualiza a tela
		m_overlay_back_surface->ReleaseDC(dc);
	}else{
		CopyBitmapToYUVSurface(fraze);	
	}

	//atualiza o overlay
	m_overlay_surface->Flip(0, DDFLIP_WAIT);
}


//==========================================================
//mostra o overlay
//==========================================================
void overlay_renderer_t::ShowOverlay(){
	RECT dest_rect;
	dest_rect.left   = OVERLAY_X;
	dest_rect.top    = OVERLAY_Y;
	dest_rect.right  = OVERLAY_X+OVERLAY_WIDTH+12;
	dest_rect.bottom = OVERLAY_Y+OVERLAY_HEIGHT+12;

	// Setup overlay flags.
	m_overlay_surface->UpdateOverlay(0, m_primary_surface, &dest_rect, DDOVER_SHOW | DDOVER_ADDDIRTYRECT, 0);
	m_overlay_surface->UpdateOverlayZOrder(DDOVERZ_SENDTOFRONT, 0);	
}


//****************************************************************************
//* Function: CopyBitmapToYUVSurface
//* Description: 
//* Copies an RGB GDI bitmap to a YUV surface. Both bitmap and surface
//* must be a multiple of 2 pixels in width for the supported YUV formats. 
//* The following formats are supported:
//* YUY2
//* UYVY
//* 
//* The "YUY2" YUV pixel format looks like this:
//* As a series of BYTES: [Y0][U][Y1][V] (reverse it for a DWORD)
//*
//* The "UYVY" YUV pixel format looks like this:
//* As a series of BYTES: [U][Y0][V][Y1] (reverse it for a DWORD)
//*
//* As you can see, both formats pack two pixels into a single DWORD. The 
//* pixels share U and V components and have separate Y components.
//* 
//* Returns: TRUE if successful, otherwise FALSE.
//****************************************************************************
BOOL overlay_renderer_t::CopyBitmapToYUVSurface(const char* fraze){//(LPDIRECTDRAWSURFACE7 lpDDSurf, HBITMAP hbm){
	#define INIT_DIRECTDRAW_STRUCT(x) (ZeroMemory(&x, sizeof(x)), x.dwSize=sizeof(x))
	
	HRESULT hr;
	DDSURFACEDESC2 ddsd;
	DWORD x, y, dwWidth, dwHeight;
	LONG lPitch;
	LPBYTE pSurf;
	DWORD dwBytesInRow;
	COLORREF color;
	BYTE R,G,B, Y0,Y1,U,V;

        
	HBITMAP hBMP;
	HDC hDCMemory;
//	HDC hDCSrc;
	
//	hDCSrc=CreateCompatibleDC(NULL);//GetDC(0);	
//	if(hDCSrc == NULL){
//		MessageBoxA(NULL,"Erro0","erro",0);
//		return FALSE;
//	}

	hDCMemory = CreateCompatibleDC(NULL);
	//hBMP = CreateCompatibleBitmap(hDCMemory, OVERLAY_WIDTH, OVERLAY_HEIGHT);

	BITMAPINFO bi24BitInfo;
	bi24BitInfo.bmiHeader.biBitCount = 24;
    bi24BitInfo.bmiHeader.biCompression = BI_RGB;
    bi24BitInfo.bmiHeader.biPlanes = 1;
    bi24BitInfo.bmiHeader.biSize = sizeof(bi24BitInfo.bmiHeader);
    bi24BitInfo.bmiHeader.biWidth = OVERLAY_WIDTH;
    bi24BitInfo.bmiHeader.biHeight = OVERLAY_HEIGHT;
	hBMP=CreateDIBSection(hDCMemory, &bi24BitInfo, DIB_RGB_COLORS, 0, 0, 0);
	SelectObject(hDCMemory, hBMP);
//============teste===============
	HFONT mfont;
	RECT RT;
	HBRUSH cor_ret1, cor_ret2;

	//define as cores dos retangulos
	cor_ret1=CreateSolidBrush(0x000066FF);	//laranja
	cor_ret2=CreateSolidBrush(0x00000000);	//preto

	//imprime o primeiro retangulo
	SetRect(&RT, 0, 0, OVERLAY_WIDTH, OVERLAY_HEIGHT);
	FillRect(hDCMemory, &RT, cor_ret1);
	
	//imprime o segundo retangulo, para o primeiro dar uma impressão de ser bordas
	SetRect(&RT, 2, 2, OVERLAY_WIDTH-2, OVERLAY_HEIGHT-2);
	FillRect(hDCMemory, &RT, cor_ret2);

	//cria a fonte a ser usado
	mfont=CreateFontA(OVERLAY_HEIGHT, 0, 0, 0, FW_BOLD, false, false, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, DEFAULT_PITCH, "Arial");
	SelectObject(hDCMemory, mfont);

	//atribui a cor da fonte e fundo
	SetTextColor(hDCMemory, 0x0000CCFF);	//laranja
	SetBkColor(hDCMemory, 0x00000000);		//preto

	//imprime o texto na posicao informada
	DrawText(hDCMemory, fraze, strlen(fraze), &RT, DT_CENTER);

	//delete a font anterior	
	if(mfont){DeleteObject(SelectObject(hDCMemory, mfont));}

//============fim teste===============	
	//SelectObject(hDCMemory, hBMP);
	//BitBlt(hDCMemory, 0, 0, OVERLAY_WIDTH, OVERLAY_HEIGHT, hDCSrc, 0, 0, SRCCOPY);
	
//	if(hDCSrc !=NULL){DeleteDC(hDCSrc);}	//limpa hdc

	if(hBMP == NULL){
		return FALSE;
	}

	INIT_DIRECTDRAW_STRUCT(ddsd);
	//Lock down the surface so we can modify it's contents.
	hr=m_overlay_back_surface->Lock( NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT, NULL);
	if (FAILED(hr)){
		goto limpa;
	}

	dwWidth=ddsd.dwWidth;
	dwHeight=ddsd.dwHeight;
	lPitch=ddsd.lPitch;
	pSurf=(LPBYTE)ddsd.lpSurface;
	dwBytesInRow=ddsd.dwWidth*2;

	// Go through the image 2 pixels at a time and convert to YUV
	for(y=0; y<dwHeight;y++) {
		for(x=0; x<dwWidth;x+=2){
			// The equations for color conversion used here, probably aren't exact, but they seem to do an OK job.
			color=GetPixel(hDCMemory, x,y);
			R=GetRValue(color);
			G=GetGValue(color);
			B=GetBValue(color);
			Y0= (BYTE)(0.29*R + 0.59*G + 0.14*B);			
			U= (BYTE)(128.0 - 0.14*R - 0.29*G + 0.43*B);

			color=GetPixel(hDCMemory, x+1,y);
			R=GetRValue(color);
			G=GetGValue(color);
			B=GetBValue(color);
			Y1= (BYTE)(0.29*R + 0.57*G + 0.14*B);
			V= (BYTE)(128.0 + 0.36*R - 0.29*G - 0.07*B);

			switch (ddsd.ddpfPixelFormat.dwFourCC)
			{
				case MAKEFOURCC('Y','U','Y','2'): 
					*(pSurf++) = Y0;
					*(pSurf++) = U;
					*(pSurf++) = Y1;
					*(pSurf++) = V;
					break;
				case MAKEFOURCC('U','Y','V','Y'): 
					*(pSurf++) = U;
					*(pSurf++) = Y0;
					*(pSurf++) = V;
					*(pSurf++) = Y1;
					break;
			} 
		}
		pSurf+=(lPitch-dwBytesInRow);
	}

	m_overlay_back_surface->Unlock(NULL); 

	limpa:
		if(hDCMemory !=NULL){DeleteDC(hDCMemory);}
//		if(hDCSrc !=NULL){DeleteDC(hDCSrc);}
		if(hBMP !=NULL){DeleteObject(hBMP);}

	return TRUE;
}



//==========================================================
//deleta os objetos
//==========================================================
void overlay_renderer_t::CleanUp(){
	m_overlay_surface= 0;
	m_primary_surface= 0;
	m_surfaces_lost = false;
}


//desabilita o desktopmanager
void overlay_renderer_t::DisableDesktopWindowManager(){
	HMODULE dwmModule = LoadLibraryA("dwmapi.dll");
	if(dwmModule){
		typedef HRESULT (STDAPICALLTYPE *DwmIsCompositionEnabledFunc)(BOOL* enabled);
		typedef HRESULT (STDAPICALLTYPE *DwmEnableCompositionFunc)(UINT action);

		DwmIsCompositionEnabledFunc IsCompositionEnabled =
			reinterpret_cast<DwmIsCompositionEnabledFunc>(
			GetProcAddress(dwmModule, "DwmIsCompositionEnabled"));
			
		BOOL enabled;
		IsCompositionEnabled(&enabled);
		if(enabled){
			DwmEnableCompositionFunc EnableComposition = 
				reinterpret_cast<DwmEnableCompositionFunc>(
				GetProcAddress(dwmModule, "DwmEnableComposition"));
			EnableComposition(0);
		}
		FreeLibrary(dwmModule);
	}
}

//retorna a versão do sistema operacional
unsigned long overlay_renderer_t::get_versao(void){
	return WIN_VERSAO;
}