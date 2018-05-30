#include "gpk_gui.h"
#include "gpk_bitmap_target.h"
#include "gpk_grid_copy.h"

			::gpk::error_t								gpk::controlCreate							(::gpk::SGUI& gui)										{
	::gpk::error_t												iControl									= -1;
	gpk_necall(iControl = ::gpk::resize( gui.Controls.size() + 1
		, gui.Controls
		, gui.ControlStates
		, gui.ControlMetrics
		, gui.ControlText
		, gui.ControlChildren
		, gui.ControlConstraints
		) - 1, "Failed to resize! Out of memory?");
	gui.ControlStates		[iControl]						= {};
	gui.ControlStates		[iControl].Outdated				= true;
	gui.Controls			[iControl]						= {};
	gui.Controls			[iControl].IndexParent			= -1;
	gui.Controls			[iControl].Align				= ::gpk::ALIGN_TOP_LEFT;
	gui.Controls			[iControl].Area					= {{0, 0}, {16, 16}};
	gui.ControlConstraints	[iControl]						= {-1, -1};
	return iControl; 
}

			::gpk::error_t								gpk::controlDelete							(::gpk::SGUI& gui, int32_t iControl)					{ 
	ree_if((gui.Controls.size() <= uint32_t(iControl)), "Invalid control id: %u.", iControl);
	::gpk::SControlState										& controlState								= gui.ControlStates[iControl];
	ree_if(controlState.Unused, "Invalid control id: %u.", iControl);
	controlState.Unused										= true;
	const uint32_t												indexParent									= (uint32_t)gui.Controls[iControl].IndexParent;
	if(indexParent < gui.Controls.size() && false == gui.ControlStates[indexParent].Unused) {
		::gpk::array_pod<int32_t>									& children									= gui.ControlChildren[indexParent];
		for(uint32_t iChild = 0, countChild = children.size(); iChild < countChild; ++iChild)
			if(children[iChild] == iControl) {
				gpk_necall(children.remove(iChild), "Failed to remove child at index: %u.", iChild);
				break;
			}
	}
	::gpk::array_view<int32_t>									& children									= gui.ControlChildren[iControl];
	for(uint32_t iChild = 0, countChild = children.size(); iChild < countChild; ++iChild)
		gui.ControlStates[children[iChild]].Unused				= true;
	return 0; 
}

	::gpk::error_t										gpk::controlSetParent						(::gpk::SGUI& gui, int32_t iControl, int32_t iParent)	{
	ree_if((gui.Controls.size() <= uint32_t(iControl)) || gui.ControlStates[iControl].Unused, "Invalid control id: %u.", iControl);
	::gpk::SControl												& control									= gui.Controls[iControl];
	if(control.IndexParent == iParent)	// Exit early if there is nothing to do here.
		return 0;

	control.IndexParent										= iParent;
	if(gui.Controls.size() <= ((uint32_t)iParent) || gui.ControlStates[iParent].Unused)
		return 0;

	// Set iControl to parent's children array.
	::gpk::array_pod<int32_t>									& children									= gui.ControlChildren[iParent];
	for(uint32_t iChild = 0, countChild = children.size(); iChild < countChild; ++iChild)
		if(children[iChild] == iControl)
			return 0;

	gpk_necall(children.push_back(iControl), "Out of memory?");
	return 0;
}

	static		::gpk::error_t								controlUpdateMetrics						(::gpk::SGUI& gui, int32_t iControl, ::gpk::grid_view<::gpk::SColorBGRA>& target)					{
	ree_if((gui.Controls.size() <= uint32_t(iControl)), "Invalid control id: %u.", iControl);
	::gpk::SControlState										& controlState								= gui.ControlStates[iControl];
	ree_if(controlState.Unused, "Invalid control id: %u.", iControl);
	const ::gpk::SControl										& control									= gui.Controls[iControl];
	::gpk::SCoord2<int32_t>										targetSize									= target.metrics().Cast<int32_t>();
	const bool													isValidParent								= gui.Controls.size() > (uint32_t)control.IndexParent && false == gui.ControlStates[control.IndexParent].Unused;
	if(isValidParent) {
		::controlUpdateMetrics(gui, control.IndexParent, target);
		targetSize												= gui.ControlMetrics[control.IndexParent].Client.Global.Size;
	}
	if(false == controlState.Outdated) 
		return 0;

	::gpk::SCoord2<double>										scale										= gui.Zoom.DPI * gui.Zoom.ZoomLevel;
	::gpk::SCoord2<int32_t>										finalPosition								= ::gpk::SCoord2<double>{control.Area.Offset	.x * scale.x, control.Area.Offset	.y * scale.y}.Cast<int32_t>();
	::gpk::SCoord2<int32_t>										finalSize									= ::gpk::SCoord2<double>{control.Area.Size		.x * scale.x, control.Area.Size		.y * scale.y}.Cast<int32_t>();
	::gpk::SControlMetrics										& controlMetrics							= gui.ControlMetrics[iControl];
	const ::gpk::SControlConstraints							& controlConstraints						= gui.ControlConstraints[iControl];
	if(controlConstraints.IndexControlToAttachWidthTo	== iControl) { finalPosition.x = 0; finalSize.x = targetSize.x; } else if(gui.Controls.size() > (uint32_t)controlConstraints.IndexControlToAttachWidthTo  && false == gui.ControlStates[controlConstraints.IndexControlToAttachWidthTo ].Unused) { finalPosition.x = 0; finalSize.x = gui.ControlMetrics[controlConstraints.IndexControlToAttachWidthTo ].Total.Global.Size.x; }
	if(controlConstraints.IndexControlToAttachHeightTo	== iControl) { finalPosition.y = 0; finalSize.y = targetSize.y; } else if(gui.Controls.size() > (uint32_t)controlConstraints.IndexControlToAttachHeightTo && false == gui.ControlStates[controlConstraints.IndexControlToAttachHeightTo].Unused) { finalPosition.y = 0; finalSize.y = gui.ControlMetrics[controlConstraints.IndexControlToAttachHeightTo].Total.Global.Size.y; }
	controlMetrics.Client.Local								= 
		{	::gpk::SCoord2<double>{(control.Margin.Left + control.Border.Left) * scale.x, (control.Margin.Top + control.Border.Top) * scale.y}.Cast<int32_t>()
		,	{ (int32_t)(finalSize.x - (control.Margin.Left	+ control.Border.Left	+ control.Margin.Right	+ control.Border.Right)		* scale.x)
			, (int32_t)(finalSize.y - (control.Margin.Top	+ control.Border.Top	+ control.Margin.Bottom	+ control.Border.Bottom)	* scale.y)
			}
		};

	controlMetrics.Total	.Local							= {finalPosition, finalSize};
		 if(::gpk::bit_true(control.Align, ::gpk::ALIGN_HCENTER	))	{ controlMetrics.Total.Local.Offset.x = targetSize.x / 2 - finalSize.x / 2 + finalPosition.x; }
	else if(::gpk::bit_true(control.Align, ::gpk::ALIGN_RIGHT	))	{ controlMetrics.Total.Local.Offset.x = targetSize.x - (finalSize.x + finalPosition.x); }
	else															{}

		 if(::gpk::bit_true(control.Align, ::gpk::ALIGN_VCENTER	))	{ controlMetrics.Total.Local.Offset.y = targetSize.y / 2 - finalSize.y / 2 + finalPosition.y; }
	else if(::gpk::bit_true(control.Align, ::gpk::ALIGN_BOTTOM	))	{ controlMetrics.Total.Local.Offset.y = targetSize.y - (finalSize.y + finalPosition.y); }
	else															{}

	controlMetrics.Total	.Global							= controlMetrics.Total	.Local;
	controlMetrics.Client	.Global							= controlMetrics.Client	.Local;
	controlMetrics.Client	.Global.Offset					+= controlMetrics.Total.Local.Offset;
	if(isValidParent) {
		::gpk::SControlMetrics										& parentMetrics								= gui.ControlMetrics[control.IndexParent];
		controlMetrics.Client	.Global.Offset					+= parentMetrics.Client.Global.Offset;
		controlMetrics.Total	.Global.Offset					+= parentMetrics.Client.Global.Offset;
		return 0;
	}
	return 0;
}

static		::gpk::error_t								controlTextDraw							(::gpk::SGUI& gui, int32_t iControl, ::gpk::grid_view<::gpk::SColorBGRA>& target)						{
	::gpk::SControlText											& controlText							= gui.ControlText[iControl];
	//::gpk::SControl												& control								= gui.Controls[iControl];
	::gpk::SControlState										& controlState							= gui.ControlStates[iControl];
	::gpk::SControlMetrics										& controlMetrics						= gui.ControlMetrics[iControl];
	if(0 == gui.FontTexture.Texels.size() || 0 == gui.FontTexture.Pitch)
		return 0;
	::gpk::array_pod<::gpk::SCoord2<uint32_t>>					dstCoords;
	const uint32_t												charsPerRow								= gui.FontTexture.Pitch / gui.FontCharSize.x;
	const ::gpk::SColorBGRA										fontColor								= controlState.Hover ? ::gpk::ORANGE : ::gpk::WHITE;	
	const ::gpk::SRectangle2D<int32_t>							& targetRect							= controlMetrics.Client.Global;

	::gpk::SRectangle2D<int32_t>								rectText								= {};
	rectText.Size											= {(int32_t)gui.FontCharSize.x * (int32_t)controlText.Text.size(), (int32_t)gui.FontCharSize.y};
	rectText.Offset											= targetRect.Offset;

		 if (::gpk::bit_true(controlText.Align, ::gpk::ALIGN_HCENTER)) { rectText.Offset.x = (targetRect.Offset.x + targetRect.Size.x / 2) - rectText.Size.x / 2;	}
	else if (::gpk::bit_true(controlText.Align, ::gpk::ALIGN_RIGHT	)) { rectText.Offset.x = (targetRect.Offset.x + targetRect.Size.x) - rectText.Size.x;			}

		 if (::gpk::bit_true(controlText.Align, ::gpk::ALIGN_VCENTER)) { rectText.Offset.y = (targetRect.Offset.y + targetRect.Size.y / 2) - rectText.Size.y / 2;	}
	else if (::gpk::bit_true(controlText.Align, ::gpk::ALIGN_BOTTOM	)) { rectText.Offset.y = (targetRect.Offset.y + targetRect.Size.y) - rectText.Size.y;			}

	for(uint32_t iChar = 0, countChars = (uint32_t)controlText.Text.size(); iChar < countChars; ++iChar) {
		char														charToDraw								= controlText.Text[iChar];
		const int32_t												coordTableX								= charToDraw % charsPerRow;
		const int32_t												coordTableY								= charToDraw / charsPerRow;
		const ::gpk::SCoord2<int32_t>								coordCharTable							= ::gpk::SCoord2<uint32_t>{coordTableX * gui.FontCharSize.x, coordTableY * gui.FontCharSize.y}.Cast<int32_t>();
		const ::gpk::SCoord2<int32_t>								dstOffset1								= {(int32_t)(rectText.Offset.x + gui.FontCharSize.x * iChar), rectText.Offset.y};
		const ::gpk::SRectangle2D<int32_t>							srcRect0								= {coordCharTable, gui.FontCharSize.Cast<int32_t>()};
		//error_if(errored(::gpk::grid_copy_alpha_bit(target, gui.FontTexture.View, dstOffset1, {charsPerRow * gui.FontCharSize.x, 8 * gui.FontCharSize.y}, fontColor, srcRect0)), "I believe this never fails.");
		dstCoords.clear();
		error_if(errored(::gpk::grid_raster_alpha_bit(target, gui.FontTexture.View, dstOffset1, {charsPerRow * gui.FontCharSize.x, 8 * gui.FontCharSize.y}, srcRect0, dstCoords)), "I believe this never fails.");
		for(uint32_t iCoord = 0; iCoord < dstCoords.size(); ++iCoord)
			::gpk::drawPixelLight(target, dstCoords[iCoord], fontColor, 0.05f, 0.75);
	}
	gui, iControl, target;
	return 0;
}

static		::gpk::error_t								actualControlDraw							(::gpk::SGUI& gui, int32_t iControl, ::gpk::grid_view<::gpk::SColorBGRA>& target)					{
	ree_if((gui.Controls.size() <= uint32_t(iControl)), "Invalid control id: %u.", iControl);
	::controlUpdateMetrics(gui, iControl, target);
	const ::gpk::SControlState									& controlState								= gui.ControlStates[iControl];
	const ::gpk::SControl										& control									= gui.Controls[iControl];
	::gpk::SColorBGRA											colors		[::gpk::GUI_CONTROL_AREA_COUNT]; // -- Fill color table
	colors[::gpk::GUI_CONTROL_AREA_BACKGROUND		]		= (control.ColorBack			>= gui.Colors.size()) ? ::gpk::SColorBGRA{::gpk::MAGENTA		} : gui.Colors[control.ColorBack			];
	colors[::gpk::GUI_CONTROL_AREA_CLIENT			]		= (control.ColorClient			>= gui.Colors.size()) ? ::gpk::SColorBGRA{::gpk::DARKMAGENTA	} : gui.Colors[control.ColorClient			];
	//colors[::gpk::GUI_CONTROL_AREA_BACKGROUND		]		= (control.ColorBack			>= gui.Colors.size()) ? ::gpk::SColorBGRA{::gpk::MAGENTA		} : controlState.Hover ? controlState.Pressed ? ::gpk::SColorBGRA{::gpk::LIGHTGRAY		} : ::gpk::SColorBGRA{::gpk::DARKGRAY	} : gui.Colors[control.ColorBack			];
	//colors[::gpk::GUI_CONTROL_AREA_CLIENT			]		= (control.ColorClient			>= gui.Colors.size()) ? ::gpk::SColorBGRA{::gpk::DARKMAGENTA	} : controlState.Hover ? controlState.Pressed ? ::gpk::SColorBGRA{::gpk::LIGHTORANGE	} : ::gpk::SColorBGRA{::gpk::DARKORANGE	} : gui.Colors[control.ColorClient			];
	if(true) { // 3d borders
		colors[::gpk::GUI_CONTROL_AREA_CLIENT			]		= controlState.Hover ? controlState.Pressed ? colors[::gpk::GUI_CONTROL_AREA_CLIENT		] * 0.6 : colors[::gpk::GUI_CONTROL_AREA_CLIENT		] * 0.8 : colors[::gpk::GUI_CONTROL_AREA_CLIENT		] * 1.0;
		colors[::gpk::GUI_CONTROL_AREA_BORDER_LEFT		]		= controlState.Hover ? controlState.Pressed ? colors[::gpk::GUI_CONTROL_AREA_BACKGROUND	] * 0.6 : colors[::gpk::GUI_CONTROL_AREA_BACKGROUND	] * 0.8 : colors[::gpk::GUI_CONTROL_AREA_BACKGROUND	] * 1.2; 
		colors[::gpk::GUI_CONTROL_AREA_BORDER_TOP		]		= controlState.Hover ? controlState.Pressed ? colors[::gpk::GUI_CONTROL_AREA_BACKGROUND	] * 0.6 : colors[::gpk::GUI_CONTROL_AREA_BACKGROUND	] * 0.8 : colors[::gpk::GUI_CONTROL_AREA_BACKGROUND	] * 1.2;
		colors[::gpk::GUI_CONTROL_AREA_BORDER_RIGHT		]		= controlState.Hover ? controlState.Pressed ? colors[::gpk::GUI_CONTROL_AREA_BACKGROUND	] * 1.4 : colors[::gpk::GUI_CONTROL_AREA_BACKGROUND	] * 1.2 : colors[::gpk::GUI_CONTROL_AREA_BACKGROUND	] * 0.8; 
		colors[::gpk::GUI_CONTROL_AREA_BORDER_BOTTOM	]		= controlState.Hover ? controlState.Pressed ? colors[::gpk::GUI_CONTROL_AREA_BACKGROUND	] * 1.4 : colors[::gpk::GUI_CONTROL_AREA_BACKGROUND	] * 1.2 : colors[::gpk::GUI_CONTROL_AREA_BACKGROUND	] * 0.8; 
	}
	else {
		colors[::gpk::GUI_CONTROL_AREA_CLIENT			]		= (control.ColorClient			>= gui.Colors.size()) ? ::gpk::SColorBGRA{::gpk::DARKMAGENTA	} : controlState.Hover ? controlState.Pressed ? ::gpk::SColorBGRA{::gpk::LIGHTORANGE	} : ::gpk::SColorBGRA{::gpk::DARKORANGE	} : gui.Colors[control.ColorClient			];
		colors[::gpk::GUI_CONTROL_AREA_BORDER_LEFT		]		= (control.ColorBorder.Left		>= gui.Colors.size()) ? ::gpk::SColorBGRA{::gpk::CYAN			} : controlState.Hover ? controlState.Pressed ? ::gpk::SColorBGRA{::gpk::LIGHTRED		} : ::gpk::SColorBGRA{::gpk::DARKRED	} : gui.Colors[control.ColorBorder.Left		]; 
		colors[::gpk::GUI_CONTROL_AREA_BORDER_TOP		]		= (control.ColorBorder.Top		>= gui.Colors.size()) ? ::gpk::SColorBGRA{::gpk::CYAN			} : controlState.Hover ? controlState.Pressed ? ::gpk::SColorBGRA{::gpk::LIGHTRED		} : ::gpk::SColorBGRA{::gpk::DARKRED	} : gui.Colors[control.ColorBorder.Top		];
		colors[::gpk::GUI_CONTROL_AREA_BORDER_RIGHT		]		= (control.ColorBorder.Right	>= gui.Colors.size()) ? ::gpk::SColorBGRA{::gpk::CYAN			} : controlState.Hover ? controlState.Pressed ? ::gpk::SColorBGRA{::gpk::LIGHTRED		} : ::gpk::SColorBGRA{::gpk::DARKRED	} : gui.Colors[control.ColorBorder.Right	]; 
		colors[::gpk::GUI_CONTROL_AREA_BORDER_BOTTOM	]		= (control.ColorBorder.Bottom	>= gui.Colors.size()) ? ::gpk::SColorBGRA{::gpk::CYAN			} : controlState.Hover ? controlState.Pressed ? ::gpk::SColorBGRA{::gpk::LIGHTRED		} : ::gpk::SColorBGRA{::gpk::DARKRED	} : gui.Colors[control.ColorBorder.Bottom	]; 
	}
	const ::gpk::SControlMetrics								& controlMetrics							= gui.ControlMetrics[iControl];
	::gpk::SRectangle2D<int32_t>								finalRects	[::gpk::GUI_CONTROL_AREA_COUNT]	= {};
	::gpk::SRectLimits<int32_t>									scaledBorders								= {};

	scaledBorders.Left										= (int32_t)(control.Border.Left		* gui.Zoom.DPI.x * gui.Zoom.ZoomLevel);
	scaledBorders.Top										= (int32_t)(control.Border.Top		* gui.Zoom.DPI.y * gui.Zoom.ZoomLevel);
	scaledBorders.Right										= (int32_t)(control.Border.Right	* gui.Zoom.DPI.x * gui.Zoom.ZoomLevel);
	scaledBorders.Bottom									= (int32_t)(control.Border.Bottom	* gui.Zoom.DPI.y * gui.Zoom.ZoomLevel);

	finalRects[::gpk::GUI_CONTROL_AREA_BACKGROUND		]	= controlMetrics.Total.Global ; 
	finalRects[::gpk::GUI_CONTROL_AREA_CLIENT			]	= controlMetrics.Client.Global; 
	finalRects[::gpk::GUI_CONTROL_AREA_BORDER_LEFT		]	= {controlMetrics.Total.Global.Offset, ::gpk::SCoord2<int32_t>{control.Border.Left, controlMetrics.Total.Global.Size.y}}; 
	finalRects[::gpk::GUI_CONTROL_AREA_BORDER_TOP		]	= {controlMetrics.Total.Global.Offset, ::gpk::SCoord2<int32_t>{controlMetrics.Total.Global.Size.x, control.Border.Top}}; 
	finalRects[::gpk::GUI_CONTROL_AREA_BORDER_RIGHT		]	= {controlMetrics.Total.Global.Offset + ::gpk::SCoord2<int32_t>{controlMetrics.Total.Global.Size.x - control.Border.Right, 0}, ::gpk::SCoord2<int32_t>{control.Border.Right, controlMetrics.Total.Global.Size.y}};
	finalRects[::gpk::GUI_CONTROL_AREA_BORDER_BOTTOM	]	= {controlMetrics.Total.Global.Offset + ::gpk::SCoord2<int32_t>{0, controlMetrics.Total.Global.Size.y - control.Border.Bottom}, ::gpk::SCoord2<int32_t>{controlMetrics.Total.Global.Size.x, control.Border.Bottom}};

	for(uint32_t iElement = 0; iElement < ::gpk::GUI_CONTROL_AREA_COUNT; ++iElement)
		if(iElement != ::gpk::GUI_CONTROL_AREA_CLIENT)
			::gpk::drawRectangle(target, colors[iElement], finalRects[iElement]);

	// --- Draw control corners
	::gpk::SCoord2<int32_t>										startOffset									= controlMetrics.Total.Global.Offset;
	::gpk::STriangle2D<int32_t>									triangles [8]								= {};
	triangles[0]											= {startOffset, startOffset + ::gpk::SCoord2<int32_t>{control.Border.Left, control.Border.Top}, startOffset + ::gpk::SCoord2<int32_t>{control.Border.Left, 0}	};
	triangles[1]											= {startOffset, startOffset + ::gpk::SCoord2<int32_t>{0, control.Border.Top}, startOffset + ::gpk::SCoord2<int32_t>{control.Border.Left, control.Border.Top}	};

	int32_t														startOffsetX								= startOffset.x + controlMetrics.Total.Global.Size.x - control.Border.Right;
	startOffset												= {startOffsetX, controlMetrics.Total.Global.Offset.y};
	triangles[2]											= {startOffset, startOffset + ::gpk::SCoord2<int32_t>{0, control.Border.Top}, startOffset + ::gpk::SCoord2<int32_t>{control.Border.Right, 0}	};
	triangles[3]											= {startOffset + ::gpk::SCoord2<int32_t>{control.Border.Right, 0}, startOffset + ::gpk::SCoord2<int32_t>{0, control.Border.Top}, startOffset + ::gpk::SCoord2<int32_t>{control.Border.Right, control.Border.Top}	};

	int32_t														startOffsetY								= startOffset.y + controlMetrics.Total.Global.Size.y - control.Border.Bottom;
	startOffset												= {controlMetrics.Total.Global.Offset.x, startOffsetY};
	triangles[4]											= {startOffset, startOffset + ::gpk::SCoord2<int32_t>{0, control.Border.Bottom}, startOffset + ::gpk::SCoord2<int32_t>{control.Border.Left, 0}	};
	triangles[5]											= {startOffset + ::gpk::SCoord2<int32_t>{control.Border.Right, 0}, startOffset + ::gpk::SCoord2<int32_t>{0, control.Border.Top}, startOffset + ::gpk::SCoord2<int32_t>{control.Border.Right, control.Border.Top}	};

	startOffset												= controlMetrics.Total.Global.Offset + controlMetrics.Total.Global.Size - ::gpk::SCoord2<int32_t>{control.Border.Right, control.Border.Bottom};
	triangles[6]											= {startOffset, startOffset + ::gpk::SCoord2<int32_t>{control.Border.Right, control.Border.Bottom}, startOffset + ::gpk::SCoord2<int32_t>{control.Border.Right, 0}	};
	triangles[7]											= {startOffset, startOffset + ::gpk::SCoord2<int32_t>{0, control.Border.Bottom}, startOffset + ::gpk::SCoord2<int32_t>{control.Border.Right, control.Border.Bottom}	};
	int32_t														colorIndices [8]							=
		{ ::gpk::GUI_CONTROL_AREA_BORDER_TOP		
		, ::gpk::GUI_CONTROL_AREA_BORDER_LEFT		
		, ::gpk::GUI_CONTROL_AREA_BORDER_TOP		
		, ::gpk::GUI_CONTROL_AREA_BORDER_RIGHT	
		, ::gpk::GUI_CONTROL_AREA_BORDER_LEFT		
		, ::gpk::GUI_CONTROL_AREA_BORDER_BOTTOM	
		, ::gpk::GUI_CONTROL_AREA_BORDER_RIGHT	
		, ::gpk::GUI_CONTROL_AREA_BORDER_BOTTOM	
		};
	for(uint32_t iTri = 0; iTri < 8; ++iTri)
		::gpk::drawTriangle(target, colors[colorIndices[iTri]], triangles[iTri]);

	::controlTextDraw(gui, iControl, target);
	return 0;
}
			::gpk::error_t								gpk::controlDrawHierarchy					(::gpk::SGUI& gui, int32_t iControl, ::gpk::grid_view<::gpk::SColorBGRA>& target)					{
	ree_if((gui.Controls.size() <= uint32_t(iControl)), "Invalid control id: %u.", iControl);
	::gpk::SControlState										& controlState								= gui.ControlStates[iControl];
	ree_if(controlState.Unused, "Invalid control id: %u.", iControl);
	::actualControlDraw(gui, iControl, target);
	::gpk::array_view<int32_t>									& children									= gui.ControlChildren[iControl];
	for(uint32_t iChild = 0, countChild = children.size(); iChild < countChild; ++iChild) {
		::gpk::controlDrawHierarchy(gui, children[iChild], target);
	}
	return 0;
}

static		::gpk::error_t								updateGUIControlHovered						(::gpk::SControlState& controlFlags, const ::gpk::SInput& inputSystem)	noexcept	{ 
	if(controlFlags.Hover) {
		if(inputSystem.ButtonDown(0) && false == controlFlags.Pressed) 
			controlFlags.Pressed									= true;
		else if(inputSystem.ButtonUp(0) && controlFlags.Pressed) {
			controlFlags.Execute									= true;
			controlFlags.Pressed									= false;
		}
	}
	else 
		controlFlags.Hover										= true;
	return controlFlags.Hover;
}

static		::gpk::error_t								controlProcessInput							(::gpk::SGUI& gui, ::gpk::SInput& input, int32_t iControl)							{
	::gpk::SControlState										& controlState							= gui.ControlStates[iControl];
	// EXECUTE only lasts one tick.
	if (controlState.Execute)
		controlState.Execute									= false;
	//--------------------
	::gpk::error_t												controlHovered							= -1;
	if(::gpk::in_range(gui.CursorPos.Cast<int32_t>(), gui.ControlMetrics[iControl].Total.Global)) {
		controlHovered											= ::updateGUIControlHovered(controlState, input) ? iControl : -1;
	}
	else {
		if (controlState.Hover) 
			controlState.Hover										= false;

		if(input.ButtonUp(0) && controlState.Pressed)
			controlState.Pressed									= false;
	}
	{
		::gpk::array_view<int32_t>									& children									= gui.ControlChildren[iControl];
		for(uint32_t iChild = 0, countChild = children.size(); iChild < countChild; ++iChild) {
			::gpk::error_t												controlPressed								= ::controlProcessInput(gui, input, children[iChild]);
			if(gui.Controls.size() > (uint32_t)controlPressed) {
				controlState.Hover										= false;
				controlHovered											= controlPressed;
			}
		}
	}
	return controlHovered;
}

			::gpk::error_t								gpk::guiProcessInput						(::gpk::SGUI& gui, ::gpk::SInput& input)											{
	gui.CursorPos											+= {(float)input.MouseCurrent.Deltas.x, (float)input.MouseCurrent.Deltas.y};
	::gpk::error_t												controlHovered								= -1;
	for(uint32_t iControl = 0, countControls = gui.Controls.size(); iControl < countControls; ++iControl) {
		::gpk::SControlState										& controlState								= gui.ControlStates[iControl];
		if(controlState.Unused || controlState.Disabled)
			continue;
		::gpk::SControl												& control									= gui.Controls[iControl];
		if(gui.Controls.size() > (uint32_t)control.IndexParent)	// Only process root parents
			continue;
		::gpk::error_t												controlPressed								= ::controlProcessInput(gui, input, iControl);
		if(gui.Controls.size() > (uint32_t)controlPressed && false == gui.ControlStates[iControl].Unused)
			controlHovered											= controlPressed;
	}
	if(controlHovered == -1) 
		return gui.Controls.size();
	for(uint32_t iControl = 0, countControls = gui.Controls.size(); iControl < countControls; ++iControl) {
		if(iControl != (uint32_t)controlHovered)
			gui.ControlStates[iControl].Hover						= false;
		else {
			verbose_printf("Hovered: %u.", iControl);
		}
	}
	return controlHovered;
}


