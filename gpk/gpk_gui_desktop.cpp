#include "gpk_gui_desktop.h"
#include "gpk_label.h"

#define NEW_OR_UNUSED(ctoken, utoken)																																			\
	uint32_t																	i##ctoken##									= 0;												\
	while(i##ctoken## < desktop.Items.##ctoken##s.size()) {																															\
		if(desktop.Items.##ctoken##s.Unused[i##ctoken##]) 																														\
			break;																																								\
		++i##ctoken##;																																							\
	}																																											\
	if(i##ctoken## >= desktop.Items.##ctoken##s.size())																																\
		gpk_necall(i##ctoken## = desktop.Items.##ctoken##s.push_back({}), "Out of memory?");																					\
	desktop.Items.##ctoken##s.Unused[i##ctoken##]	= false;																													\
	::gpk::S##ctoken##															& elementToSetUp							= desktop.Items.##ctoken##s[i##ctoken##]	= {};	\
	gpk_necall(::gpk::##utoken##Initialize(gui, elementToSetUp), "Unknown.");																									\
	gpk_necall(::gpk::controlSetParent(gui, elementToSetUp.IdControl, desktop.IdControl), "Invalid desktop id?");

			::gpk::error_t												gpk::desktopCreatePaletteGrid					(::gpk::SGUI& gui, ::gpk::SDesktop& desktop)										{ NEW_OR_UNUSED(PaletteGrid	, paletteGrid	); return iPaletteGrid	; }
			::gpk::error_t												gpk::desktopCreateControlList					(::gpk::SGUI& gui, ::gpk::SDesktop& desktop)										{ NEW_OR_UNUSED(ControlList	, controlList	); return iControlList	; }
			::gpk::error_t												gpk::desktopCreateViewport						(::gpk::SGUI& gui, ::gpk::SDesktop& desktop)										{ NEW_OR_UNUSED(Viewport	, viewport		); return iViewport		; }
			::gpk::error_t												gpk::desktopDeletePaletteGrid					(::gpk::SGUI& gui, ::gpk::SDesktop& desktop, int32_t iElement)						{ desktop.Items.PaletteGrids.Unused[iElement] = true; return ::gpk::controlDelete(gui, desktop.Items.PaletteGrids	[iElement].IdControl); }
			::gpk::error_t												gpk::desktopDeleteControlList					(::gpk::SGUI& gui, ::gpk::SDesktop& desktop, int32_t iElement)						{ desktop.Items.ControlLists.Unused[iElement] = true; return ::gpk::controlDelete(gui, desktop.Items.ControlLists	[iElement].IdControl); }
			::gpk::error_t												gpk::desktopDeleteViewport						(::gpk::SGUI& gui, ::gpk::SDesktop& desktop, int32_t iElement)						{ 
	desktop.Items.Viewports.Unused[iElement]										= true; 
	::gpk::SViewport															& viewport										= desktop.Items.Viewports[iElement];
	switch(viewport.ControlType) {
	case ::gpk::GUI_CONTROL_TYPE_PaletteGrid	: desktop.Items.PaletteGrids	.Unused[viewport.IdDesktopElement] = true; info_printf("Deleting viewport's palette grid: %i. Control Id: %i."	, viewport.IdDesktopElement, desktop.Items.PaletteGrids	[viewport.IdDesktopElement].IdControl);break;
	case ::gpk::GUI_CONTROL_TYPE_Viewport		: desktop.Items.Viewports		.Unused[viewport.IdDesktopElement] = true; info_printf("Deleting viewport's vp: %i. Control Id: %i."			, viewport.IdDesktopElement, desktop.Items.Viewports	[viewport.IdDesktopElement].IdControl);break;
	}
	info_printf("Deleting control for viewport: %i. Control Id: %i.", iElement, viewport.IdControl);
	return ::gpk::controlDelete(gui, viewport.IdControl); 
}


static		::gpk::error_t												displace										(::gpk::SGUI& gui, int32_t iControl, const ::gpk::SCoord2<double> & mouseDeltas)	{
	const ::gpk::SCoord2<double>												currentScale									= gui.Zoom.DPI * gui.Zoom.ZoomLevel;
	::gpk::SCoord2<double>														deltasScaled									= mouseDeltas;
	deltasScaled.Scale(1.0 / currentScale.x, 1.0 / currentScale.y);
	gui.Controls.Controls[iControl].Area.Offset								+= deltasScaled.Cast<int32_t>();
	return ::gpk::controlMetricsInvalidate(gui, iControl);
}

static		::gpk::error_t												pushToFrontAndDisplace							(::gpk::SGUI& gui, int32_t iControl, ::gpk::SInput& input)							{
	const int32_t																parentControlId									= gui.Controls.Controls[iControl].IndexParent;
	for(uint32_t iChild = 0, childCount = gui.Controls.Children[parentControlId].size(); iChild < childCount; ++iChild)
		if(gui.Controls.Children[parentControlId][iChild] == iControl) {
			gpk_necall(gui.Controls.Children[parentControlId].remove(iChild)		, "Unknown issue!");
			gpk_necall(gui.Controls.Children[parentControlId].push_back(iControl)	, "Unknown issue!");
			break;
		}
	if(input.MouseCurrent.Deltas.x || input.MouseCurrent.Deltas.y) 
		gpk_necall(::displace(gui, iControl, {(double)input.MouseCurrent.Deltas.x, (double)input.MouseCurrent.Deltas.y}), "Unknown issue!");
	return 0;
}

			::gpk::error_t												gpk::desktopUpdate						(::gpk::SGUI& gui, ::gpk::SDesktop& desktop, ::gpk::SInput& input)							{
	for(uint32_t iViewport = 0; iViewport < desktop.Items.Viewports.size(); ++iViewport) {
		if(desktop.Items.Viewports.Unused[iViewport])
			continue;

		::gpk::SViewport															& vp									= desktop.Items.Viewports[iViewport];
		if(gui.Controls.States[(uint32_t)vp.IdControls[::gpk::VIEWPORT_CONTROL_CLOSE]].Execute) 
			::gpk::desktopDeleteViewport(gui, desktop, iViewport);
		else if(gui.Controls.States[(uint32_t)vp.IdControls[::gpk::VIEWPORT_CONTROL_TITLE]].Pressed) 
			::pushToFrontAndDisplace(gui, vp.IdControl, input);
	}

	::gpk::array_pod<uint32_t>													controlsToProcess						= {};
	::gpk::guiGetProcessableControls(gui, controlsToProcess);

	bool																			inControlArea						= false;
	//bool																			anyControlPressed					= false;
	for(uint32_t iControlToProcess = 0, countControls = controlsToProcess.size(); iControlToProcess < countControls; ++iControlToProcess) {
		const uint32_t																	iControl							= controlsToProcess[iControlToProcess];
		const ::gpk::SControlState														& controlState						= gui.Controls.States[iControl];
		const ::gpk::SControlMetrics													& controlMetrics					= gui.Controls.Metrics[iControl];
		if(iControl != (uint32_t)desktop.IdControl)
			inControlArea																= inControlArea || ::gpk::in_range(gui.CursorPos.Cast<int32_t>(), controlMetrics.Total.Global);
		//if(controlState.Pressed && (int32_t)iControl != app.Desktop.IdControl)
		//	anyControlPressed															= true;

		if(controlState.Execute) {
			info_printf("Executed %u.", iControl);
			desktop.SelectedMenu														= -1;
			//for(uint32_t iMenu = 0, countMenus = desktop.Menus.size(); iMenu < countMenus; ++iMenu) {
			//	::gpk::SControlList																& menu								= desktop.ControlLists[iMenu];
			//	for(uint32_t iOption = 0, countOptions = menu.IdControls.size(); iOption < countOptions; ++iOption) {
			//		if(false == ::gpk::in_range(gui.CursorPos.Cast<int32_t>(), gui.Controls.Metrics[menu.IdControls[iOption]].Total.Global)) 
			//			gui.Controls.Constraints[menu.IdControl].Hidden								= true;
			//	}
			//}
		}
	}

	//for(uint32_t iMenu = 0, countMenus = desktop.Menus.size(); iMenu < countMenus; ++iMenu) {
	//	::gpk::SControlList															& menu									= desktop.Menus[iMenu];
	//	for(uint32_t iOption = 0, countOptions = menu.IdControls.size(); iOption < countOptions; ++iOption) {
	//		iMenu
	//	}
	//}


	//for(uint32_t iMenu0 = 0, countMenus = desktop.Menus.size() - 1; iMenu0 < countMenus; ++iMenu0) {
	//	::gpk::SControlList																& menu								= desktop.Menus[iMenu0];
	//	if(desktop.Menus.size() <= (uint32_t)menu.IndexParentList || desktop.Menus.size() <= (uint32_t)menu.IndexParentItem) 
	//		continue;
	//	const ::gpk::SControlState														& controlState						= gui.Controls.States[(uint32_t)desktop.Menus[menu.IndexParentList].IdControls[menu.IndexParentItem]]; 
	//	::gpk::SControlConstraints														& controlListConstraints			= gui.Controls.Constraints[desktop.Menus[iItem + 1].IdControl];
	//	if(controlState.Hover) {
	//		controlListConstraints.Hidden												= false;
	//		if(controlState.Execute) 
	//			desktop.SelectedMenu													= iItem;
	//	}
	//	else {
	//		const ::gpk::SControlMetrics													& controlListMetrics				= gui.Controls.Metrics[desktop.Menus[iItem + 1].IdControl];
	//		if(::gpk::in_range(gui.CursorPos.Cast<int32_t>(), controlListMetrics.Total.Global) && controlListConstraints.Hidden == false)
	//			controlListConstraints.Hidden												= false;
	//		else if(desktop.SelectedMenu != (int32_t)iItem)
	//			controlListConstraints.Hidden												= true;
	//	}
	//}


	return 0;
}

