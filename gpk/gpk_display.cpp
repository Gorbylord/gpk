#include "gpk_display.h"

		::gpk::error_t																			gpk::displayUpdateTick						(::gpk::SDisplay& displayInstance)											{ 
	::MSG																								msg											= {};
	::PeekMessage(&msg, displayInstance.PlatformDetail.WindowHandle, 0, 0, PM_REMOVE);
	::TranslateMessage	(&msg);
	::DispatchMessage	(&msg);
	if(msg.message == WM_QUIT)
		return 1;
	return 0;
}

		::gpk::error_t																			gpk::displayUpdate							(::gpk::SDisplay& displayInstance)											{ 
	::MSG																								msg											= {};
	while(::PeekMessage(&msg, displayInstance.PlatformDetail.WindowHandle, 0, 0, PM_NOREMOVE)) 
		if(1 == displayUpdateTick(displayInstance))
			return 1;
	return 0;
}

		::gpk::error_t																			drawBuffer									(::HDC hdc, int width, int height, const ::gpk::view_grid<::gpk::SColorBGRA>& colorArray)				{
	struct SOffscreenPlatformDetail { // raii destruction of resources
		uint32_t																							BitmapInfoSize								= 0;
		::BITMAPINFO																						* BitmapInfo								= 0;
		::HDC																								IntermediateDeviceContext					= 0;    // <- note, we're creating, so it needs to be destroyed
		::HBITMAP																							IntermediateBitmap							= 0;

																											~SOffscreenPlatformDetail					()																					{
			if(BitmapInfo					) ::free			(BitmapInfo					); 
			if(IntermediateBitmap			) ::DeleteObject	(IntermediateBitmap			); 
			if(IntermediateDeviceContext	) ::DeleteDC		(IntermediateDeviceContext	); 
		}
	}																									offscreenDetail								= {};

	const uint32_t																						bytesToCopy									= sizeof(::RGBQUAD) * colorArray.size();
	offscreenDetail.BitmapInfoSize																	= sizeof(::BITMAPINFO) + bytesToCopy;
	ree_if(0 == (offscreenDetail.BitmapInfo = (::BITMAPINFO*)::malloc(offscreenDetail.BitmapInfoSize)), "malloc(%u) failed! Out of memory?", offscreenDetail.BitmapInfoSize);
	const ::gpk::SCoord2<uint32_t>																		metricsSource								= colorArray.metrics();
	for(uint32_t y = 0, yMax = metricsSource.y; y < yMax; ++y)
		memcpy(&offscreenDetail.BitmapInfo->bmiColors[y * metricsSource.x], colorArray[metricsSource.y - 1 - y].begin(), metricsSource.x * sizeof(::gpk::SColorBGRA));

	offscreenDetail.BitmapInfo->bmiHeader.biSize													= sizeof(::BITMAPINFO);
	offscreenDetail.BitmapInfo->bmiHeader.biWidth													= colorArray.metrics().x;
	offscreenDetail.BitmapInfo->bmiHeader.biHeight													= colorArray.metrics().y;
	offscreenDetail.BitmapInfo->bmiHeader.biPlanes													= 1;
	offscreenDetail.BitmapInfo->bmiHeader.biBitCount												= 32;
	offscreenDetail.BitmapInfo->bmiHeader.biCompression												= BI_RGB;
	offscreenDetail.BitmapInfo->bmiHeader.biSizeImage												= bytesToCopy;
	offscreenDetail.BitmapInfo->bmiHeader.biXPelsPerMeter											= 0x0ec4; // Paint and PSP use these values.
	offscreenDetail.BitmapInfo->bmiHeader.biYPelsPerMeter											= 0x0ec4; // Paint and PSP use these values.
	offscreenDetail.BitmapInfo->bmiHeader.biClrUsed													= 0; 
	offscreenDetail.BitmapInfo->bmiHeader.biClrImportant											= 0;

	offscreenDetail.IntermediateDeviceContext														= ::CreateCompatibleDC(hdc);    // <- note, we're creating, so it needs to be destroyed
	char																								* ppvBits									= 0;
	reterr_error_if(0 == (offscreenDetail.IntermediateBitmap = ::CreateDIBSection(offscreenDetail.IntermediateDeviceContext, offscreenDetail.BitmapInfo, DIB_RGB_COLORS, (void**) &ppvBits, NULL, 0)), "Failed to create intermediate dib section.");
	reterr_error_if(0 == ::SetDIBits(NULL, offscreenDetail.IntermediateBitmap, 0, height, offscreenDetail.BitmapInfo->bmiColors, offscreenDetail.BitmapInfo, DIB_RGB_COLORS), "Cannot copy bits into dib section.");
	::HBITMAP																							hBmpOld										= (::HBITMAP)::SelectObject(offscreenDetail.IntermediateDeviceContext, offscreenDetail.IntermediateBitmap);    // <- altering state
	//error_if(FALSE == ::BitBlt(hdc, 0, 0, width, height, offscreenDetail.IntermediateDeviceContext, 0, 0, SRCCOPY), "Not sure why would this happen but probably due to mismanagement of the target size or the system resources. I've had it failing when I acquired the device too much and never released it.");
	::SetStretchBltMode(hdc, COLORONCOLOR);
	error_if(FALSE == ::StretchBlt(hdc, 0, 0, width, height, offscreenDetail.IntermediateDeviceContext, 0, 0, colorArray.metrics().x, colorArray.metrics().y, SRCCOPY), "Not sure why would this happen but probably due to mismanagement of the target size or the system resources. I've had it failing when I acquired the device too much and never released it.");
	::SelectObject(hdc, hBmpOld);	// put the old bitmap back in the DC (restore state)
	return 0;
}

		::gpk::error_t																			gpk::displayPresentTarget					(::gpk::SDisplay& displayInstance, const ::gpk::view_grid<::gpk::SColorBGRA>& targetToPresent)		{ 
	::HWND																								windowHandle								= displayInstance.PlatformDetail.WindowHandle;
	retwarn_warn_if(0 == windowHandle, "presentTarget called without a valid window handle set for the main window.");
	::HDC																								dc											= ::GetDC(windowHandle);
	reterr_error_if(0 == dc, "Failed to retrieve device context from the provided window handle.");
	error_if(errored(::drawBuffer(dc, displayInstance.Size.x, displayInstance.Size.y, targetToPresent)), "Not sure why this would happen.");
	::ReleaseDC(windowHandle, dc);
	return 0;
}
