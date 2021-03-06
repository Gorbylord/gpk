#include "gpk_image.h"
#include "gpk_color.h"
#include "gpk_coord.h"
#include <memory> // this is required for ::std::swap()

#ifndef BITMAP_TARGET_H_98237498023745654654
#define BITMAP_TARGET_H_98237498023745654654

namespace gpk
{
	template<typename _tCoord, typename _tCell>
					::gpk::error_t											drawPixelBrightness								(::gpk::view_grid<_tCell> & viewOffscreen, const ::gpk::SCoord2<_tCoord> & sourcePosition, const _tCell& colorLight, float factor, double range)								{	// --- This function will draw some coloured symbols in each cell of the ASCII screen.
		::gpk::SCoord2<double>														maxRange										= {range, range};
		double																		rangeUnit										= 1.0 / maxRange.Length();
		for(int32_t y = -(int32_t)range - 1, blendCount = 1 + (int32_t)range + 1; y < blendCount; ++y)	// the + 1 - 1 is because we actually process more surrounding pixels in order to compensate for the flooring of the coordinates 
		for(int32_t x = -(int32_t)range - 1; x < blendCount; ++x) {										// as it causes a visual effect of the light being cut to a rectangle and having sharp borders.
			if(x || y) {
				::gpk::SCoord2<_tCoord>													blendPos										= sourcePosition + ::gpk::SCoord2<_tCoord>{(_tCoord)x, (_tCoord)y};
				if( blendPos.x < (int32_t)viewOffscreen.metrics().x && blendPos.x >= 0
				 && blendPos.y < (int32_t)viewOffscreen.metrics().y && blendPos.y >= 0
				 ) {
					::gpk::SCoord2<_tCoord>													brightDistance									= blendPos - sourcePosition;
					double																	brightDistanceLength							= brightDistance.Length();
					double																	distanceFactor									= brightDistanceLength * rangeUnit;
					if(distanceFactor <= 1.0)
						viewOffscreen[(uint32_t)blendPos.y][(uint32_t)blendPos.x]			= ::gpk::interpolate_linear(viewOffscreen[(uint32_t)blendPos.y][(uint32_t)blendPos.x], colorLight, factor * (1.0f - distanceFactor));
				}
			}
		}
		return 0;
	}

	template<typename _tCoord, typename _tCell>
					::gpk::error_t											drawPixelLight									(::gpk::view_grid<_tCell> & viewOffscreen, const ::gpk::SCoord2<_tCoord> & sourcePosition, const _tCell& colorLight, float maxFactor, double range)								{	// --- This function will draw some coloured symbols in each cell of the ASCII screen.
		if( ((uint32_t)sourcePosition.x) < viewOffscreen.metrics().x
		 && ((uint32_t)sourcePosition.y) < viewOffscreen.metrics().y
		 )
			viewOffscreen[(uint32_t)sourcePosition.y][(uint32_t)sourcePosition.x]	= colorLight;
		return drawPixelBrightness(viewOffscreen, sourcePosition, colorLight, maxFactor, range);
	}

	template<typename _tElement>
						::gpk::error_t										updateSizeDependentTarget					(::gpk::array_pod<_tElement>& out_colors, ::gpk::view_grid<_tElement>& out_view, const ::gpk::SCoord2<uint32_t>& newSize)											{ 
		ree_if(errored(out_colors.resize(newSize.x * newSize.y)), "Out of memory?");		// Update size-dependent resources.
		if( out_view.metrics() != newSize) 
			out_view																= {out_colors.begin(), newSize};
		return 0;
	}

	template<typename _tElement>
	static inline		::gpk::error_t										updateSizeDependentTarget					(::gpk::SImage<_tElement>& out_texture, const ::gpk::SCoord2<uint32_t>& newSize)																					{ 
		return updateSizeDependentTarget(out_texture.Texels, out_texture.View, newSize);
	}
	template<typename _tElement>
						::gpk::error_t										updateSizeDependentTexture					(::gpk::array_pod<_tElement>& out_scaled, ::gpk::view_grid<_tElement>& out_view, const ::gpk::view_grid<_tElement>& in_view, const ::gpk::SCoord2<uint32_t>& newSize)											{ 
		ree_if(errored(out_scaled.resize(newSize.x * newSize.y)), "Out of memory?");		// Update size-dependent resources.
		if( out_view.metrics() != newSize ) { 
			out_view																= {out_scaled.begin(), newSize.x, newSize.y};
			if(in_view.size())
				error_if(errored(::gpk::grid_scale(out_view, in_view)), "I believe this never fails.");
		}
		return 0;
	}

	template<typename _tElement>
	static inline		::gpk::error_t										updateSizeDependentTexture					(::gpk::SImage<_tElement>& out_texture, const ::gpk::view_grid<_tElement>& in_view, const ::gpk::SCoord2<uint32_t>& newSize)																					{ 
		return updateSizeDependentTexture(out_texture.Texels, out_texture.View, in_view, newSize);
	}

	// This implementation is incorrect. The problem is that it draws borders even if it shuoldn't. I never tested it but I believe that's what the code says.
	template<typename _tCoord, typename _tColor>
	static					::gpk::error_t									drawRectangleBorder							(::gpk::view_grid<_tColor>& bitmapTarget, const _tColor& value, const ::gpk::SRectangle2D<_tCoord>& rectangle)		{
		int32_t																		yStart										= (int32_t)::gpk::max(0, (int32_t)rectangle.Offset.y);
		int32_t																		yStop										= ::gpk::min((int32_t)rectangle.Offset.y + (int32_t)rectangle.Size.y, (int32_t)bitmapTarget.metrics().y);
		int32_t																		xStart										= (int32_t)::gpk::max(0, (int32_t)rectangle.Offset.x);
		int32_t																		xStop										= ::gpk::min((int32_t)rectangle.Offset.x + (int32_t)rectangle.Size.x, (int32_t)bitmapTarget.metrics().x);
		if(yStart >= yStop || xStart >= xStop)
			return 0;
		for(int32_t x = xStart; x < xStop; ++x) 	
			bitmapTarget[yStart][x]													= value;
		memcpy(&bitmapTarget[yStop - 1][xStart], &bitmapTarget[yStart][xStart], sizeof(_tColor) * xStop - xStart);
		for(int32_t y = yStart + 1, yMax = (yStop - 1); y < yMax; ++y) {
			bitmapTarget[y][0]														= value;
			bitmapTarget[y][xStop - 1]												= value;
		}
		return (yStop - yStart) * 2 + (xStop - xStart) * 2;
	}

	template<typename _tCoord, typename _tColor>
	static					::gpk::error_t									drawCircle									(::gpk::view_grid<_tColor>& bitmapTarget, const _tColor& value, const ::gpk::SCircle2D<_tCoord>& circle)			{
		int32_t																		xStop										= ::gpk::min((int32_t)(circle.Center.x + circle.Radius), (int32_t)bitmapTarget.metrics().x);
		double																		radiusSquared								= circle.Radius * circle.Radius;
		int32_t																		pixelsDrawn									= 0;
		for(int32_t y = ::gpk::max(0, (int32_t)(circle.Center.y - circle.Radius)), yStop = ::gpk::min((int32_t)(circle.Center.y + circle.Radius), (int32_t)bitmapTarget.metrics().y); y < yStop; ++y)
		for(int32_t x = ::gpk::max(0, (int32_t)(circle.Center.x - circle.Radius)); x < xStop; ++x) {	
			::gpk::SCoord2<int32_t>														cellCurrent									= {x, y};
			double																		distanceSquared								= (cellCurrent - circle.Center).LengthSquared();
			if(distanceSquared < radiusSquared) {
			 	for(const int32_t xLimit = ::gpk::min((int32_t)circle.Center.x + ((int32_t)circle.Center.x - x), (int32_t)bitmapTarget.metrics().x); x < xLimit; ++x) {
					bitmapTarget[y][x]														= value;
					++pixelsDrawn;
				}
				break;
			}
		}
		return pixelsDrawn;
	}

	template<typename _tCoord, typename _tColor>
	static					::gpk::error_t									drawCircle									(const ::gpk::SCoord2<uint32_t>& targetMetrics, const ::gpk::SCircle2D<_tCoord>& circle, ::gpk::array_pod<::gpk::SCoord2<int32_t>>& out_Points)			{
		int32_t																		xStop										= ::gpk::min((int32_t)(circle.Center.x + circle.Radius), (int32_t)bitmapTarget.metrics().x);
		double																		radiusSquared								= circle.Radius * circle.Radius;
		int32_t																		pixelsDrawn									= 0;
		for(int32_t y = ::gpk::max(0, (int32_t)(circle.Center.y - circle.Radius)), yStop = ::gpk::min((int32_t)(circle.Center.y + circle.Radius), (int32_t)targetMetrics.y); y < yStop; ++y)
		for(int32_t x = ::gpk::max(0, (int32_t)(circle.Center.x - circle.Radius)); x < xStop; ++x) {	
			::gpk::SCoord2<int32_t>														cellCurrent									= {x, y};
			double																		distanceSquared								= (cellCurrent - circle.Center).LengthSquared();
			if(distanceSquared < radiusSquared) {
			 	for(const int32_t xLimit = ::gpk::min((int32_t)circle.Center.x + ((int32_t)circle.Center.x - x), (int32_t)targetMetrics.x); x < xLimit; ++x) {
					out_Points.push_back({x, y});
					++pixelsDrawn;
				}
				break;
			}
		}
		return pixelsDrawn;
	}

	// A good article on this kind of triangle rasterization: https://fgiesen.wordpress.com/2013/02/08/triangle-rasterization-in-practice/ 
	template<typename _tCoord, typename _tColor>
	static					::gpk::error_t									drawTriangle								(::gpk::view_grid<_tColor>& bitmapTarget, const _tColor& value, const ::gpk::STriangle2D<_tCoord>& triangle)										{
		::gpk::SCoord2	<int32_t>													areaMin										= {(int32_t)::gpk::min(::gpk::min(triangle.A.x, triangle.B.x), triangle.C.x), (int32_t)::gpk::min(::gpk::min(triangle.A.y, triangle.B.y), triangle.C.y)};
		::gpk::SCoord2	<int32_t>													areaMax										= {(int32_t)::gpk::max(::gpk::max(triangle.A.x, triangle.B.x), triangle.C.x), (int32_t)::gpk::max(::gpk::max(triangle.A.y, triangle.B.y), triangle.C.y)};
		const int32_t																xStop										= ::gpk::min(areaMax.x, (int32_t)bitmapTarget.metrics().x);
		int32_t																		pixelsDrawn									= 0;
		for(int32_t y = ::gpk::max(areaMin.y, 0), yStop = ::gpk::min(areaMax.y, (int32_t)bitmapTarget.metrics().y); y < yStop; ++y)
		for(int32_t x = ::gpk::max(areaMin.x, 0); x < xStop; ++x) {	
			const ::gpk::SCoord2<int32_t>												cellCurrent									= {x, y};
			// Determine barycentric coordinates
			int																			w0											= ::gpk::orient2d({triangle.B, triangle.A}, cellCurrent);	// ::gpk::orient2d({triangle.A, triangle.B}, cellCurrent);
			int																			w1											= ::gpk::orient2d({triangle.C, triangle.B}, cellCurrent);	// ::gpk::orient2d({triangle.B, triangle.C}, cellCurrent);
			int																			w2											= ::gpk::orient2d({triangle.A, triangle.C}, cellCurrent);	// ::gpk::orient2d({triangle.C, triangle.A}, cellCurrent);
			if(w0 < 0)
				continue;
			if(w1 < 0)
				continue;
			if(w2 < 0)
				continue;
			//if (w0 >= 0 && w1 >= 0 && w2 >= 0) { // If p is on or inside all edges, render pixel.
				bitmapTarget[y][x]														= value;
				++pixelsDrawn;
			//}
		}
		return pixelsDrawn;
	}

	template<typename _tCoord>
	static					::gpk::error_t									drawTriangle								(const ::gpk::SCoord2<uint32_t>& targetMetrics, const ::gpk::STriangle2D<_tCoord>& triangle, ::gpk::array_pod<::gpk::SCoord2<int32_t>>& out_Points)		{
		::gpk::SCoord2	<int32_t>													areaMin										= {(int32_t)::gpk::min(::gpk::min(triangle.A.x, triangle.B.x), triangle.C.x), (int32_t)::gpk::min(::gpk::min(triangle.A.y, triangle.B.y), triangle.C.y)};
		::gpk::SCoord2	<int32_t>													areaMax										= {(int32_t)::gpk::max(::gpk::max(triangle.A.x, triangle.B.x), triangle.C.x), (int32_t)::gpk::max(::gpk::max(triangle.A.y, triangle.B.y), triangle.C.y)};
		const int32_t																xStop										= ::gpk::min(areaMax.x, (int32_t)targetMetrics.x);
		int32_t																		pixelsDrawn									= 0;
		for(int32_t y = ::gpk::max(areaMin.y, 0), yStop = ::gpk::min(areaMax.y, (int32_t)targetMetrics.y); y < yStop; ++y)
		for(int32_t x = ::gpk::max(areaMin.x, 0); x < xStop; ++x) {	
			const ::gpk::SCoord2<int32_t>												cellCurrent									= {x, y};
			// Determine barycentric coordinates
			int32_t																		w0											= ::gpk::orient2d({triangle.B, triangle.A}, cellCurrent);	// ::gpk::orient2d({triangle.A, triangle.B}, cellCurrent);
			int32_t																		w1											= ::gpk::orient2d({triangle.C, triangle.B}, cellCurrent);	// ::gpk::orient2d({triangle.B, triangle.C}, cellCurrent);
			int32_t																		w2											= ::gpk::orient2d({triangle.A, triangle.C}, cellCurrent);	// ::gpk::orient2d({triangle.C, triangle.A}, cellCurrent);
			if(w0 < 0)
				continue;
			if(w1 < 0)
				continue;
			if(w2 < 0)
				continue;
			//if (w0 >= 0 && w1 >= 0 && w2 >= 0) { // If p is on or inside all edges, render pixel.
			out_Points.push_back({x, y});
			++pixelsDrawn;
			//}
		}
		return pixelsDrawn;
	}

	template<typename _tCoord>
	static					::gpk::error_t									drawTriangle								(::gpk::view_grid<uint32_t>& targetDepth, double fFar, double fNear, const ::gpk::STriangle3D<_tCoord>& triangle, ::gpk::array_pod<::gpk::SCoord2<int32_t>>& out_Points, ::gpk::array_pod<::gpk::STriangleWeights<double>>& triangleWeigths)		{
		int32_t																		pixelsDrawn									= 0;
		const ::gpk::SCoord2<uint32_t>												& _targetMetrics							= targetDepth.metrics();
		::gpk::SCoord2	<float>														areaMin										= {(float)::gpk::min(::gpk::min(triangle.A.x, triangle.B.x), triangle.C.x), (float)::gpk::min(::gpk::min(triangle.A.y, triangle.B.y), triangle.C.y)};
		::gpk::SCoord2	<float>														areaMax										= {(float)::gpk::max(::gpk::max(triangle.A.x, triangle.B.x), triangle.C.x), (float)::gpk::max(::gpk::max(triangle.A.y, triangle.B.y), triangle.C.y)};
		const float																	xStop										= ::gpk::min(areaMax.x, (float)_targetMetrics.x);
		for(float y = ::gpk::max(areaMin.y, 0.f), yStop = ::gpk::min(areaMax.y, (float)_targetMetrics.y); y < yStop; ++y)
		for(float x = ::gpk::max(areaMin.x, 0.f); x < xStop; ++x) {	
			const ::gpk::SCoord2<int32_t>												cellCurrent									= {(int32_t)x, (int32_t)y};
			const ::gpk::STriangle2D<int32_t>											triangle2D									= 
				{ {(int32_t)triangle.A.x, (int32_t)triangle.A.y}
				, {(int32_t)triangle.B.x, (int32_t)triangle.B.y}
				, {(int32_t)triangle.C.x, (int32_t)triangle.C.y}
				};
			{
				int32_t																		w0											= ::gpk::orient2d({triangle2D.B, triangle2D.A}, cellCurrent);	// Determine barycentric coordinates
				int32_t																		w1											= ::gpk::orient2d({triangle2D.C, triangle2D.B}, cellCurrent);
				int32_t																		w2											= ::gpk::orient2d({triangle2D.A, triangle2D.C}, cellCurrent);
				if(w0 <= -1 || w1 <= -1 || w2 <= -1) // ---- If p is on or inside all edges, render pixel.
					continue;
			}
			const ::gpk::SCoord2<double>												cellCurrentF								= {x, y};
			::gpk::STriangleWeights<double>												proportions									= 
				{ ::gpk::orient2d3d({triangle.C.Cast<double>(), triangle.B.Cast<double>()}, cellCurrentF)
				, ::gpk::orient2d3d({triangle.A.Cast<double>(), triangle.C.Cast<double>()}, cellCurrentF)
				, ::gpk::orient2d3d({triangle.B.Cast<double>(), triangle.A.Cast<double>()}, cellCurrentF)	// Determine barycentric coordinates
				};
			double																		proportABC									= proportions.A + proportions.B + proportions.C;
			if(proportABC == 0)
				continue;
			proportions.A															/= proportABC;
			proportions.B															/= proportABC;
			proportions.C															/= proportABC;
			double																		finalZ
				= triangle.A.z * proportions.A
				+ triangle.B.z * proportions.B
				+ triangle.C.z * proportions.C
				;
			double																		depth										= ((finalZ - fNear) / (fFar - fNear));
			if(depth > 1 || depth < 0) // discard from depth planes
				continue;
			uint32_t																	finalDepth									= (uint32_t)(depth * 0x00FFFFFFU);
			if (targetDepth[(uint32_t)y][(uint32_t)x] > (uint32_t)finalDepth) { // check against depth buffer
				targetDepth[(uint32_t)y][(uint32_t)x]									= finalDepth;
				triangleWeigths.push_back(proportions);
				out_Points.push_back(cellCurrent);
				++pixelsDrawn;
			}
		}
		return pixelsDrawn;
	}

	template<typename _tCoord>
	static	inline			::gpk::error_t									drawTriangleIndexed							
		( ::gpk::view_grid<uint32_t>						& targetDepth
		, double											fFar
		, double											fNear
		, uint32_t											baseIndex
		, uint32_t											baseVertexIndex
		, const ::gpk::view_array<::gpk::SCoord3<_tCoord>>	& coordList
		, const ::gpk::view_array<uint32_t>					& indices
		, ::gpk::array_pod<::gpk::SCoord2<int32_t>>			& out_Points
		, ::gpk::array_pod<::gpk::STriangleWeights<double>>	& triangleWeigths
		) {
		return drawTriangle(targetDepth, fFar, fNear, {coordList[indices[baseIndex + 0]], coordList[indices[baseIndex + 1]], coordList[indices[baseIndex + 2]]}, out_Points, triangleWeigths);
	}

	typedef		::gpk::error_t												(*gpk_raster_callback)						(void* bitmapTarget, const ::gpk::SCoord2<uint32_t>& bitmapMetrics, const ::gpk::SCoord2<uint32_t>& cellPos, const void* value);

	// Bresenham's line algorithm
	template<typename _tCoord, typename _tColor>
	static					::gpk::error_t									rasterLine									(::gpk::view_grid<_tColor>& bitmapTarget, const _tColor& value, const ::gpk::SLine2D<_tCoord>& line, gpk_raster_callback callback)				{
		::gpk::SCoord2<float>														A											= line.A.Cast<float>();
		::gpk::SCoord2<float>														B											= line.B.Cast<float>();
		const bool																	steep										= (::fabs(B.y - A.y) > ::fabs(B.x - A.x));
		if(steep){
			::std::swap(A.x, A.y);
			::std::swap(B.x, B.y);
		}
		if(A.x > B.x) {
			::std::swap(A.x, B.x);
			::std::swap(A.y, B.y);
		}
		const ::gpk::SCoord2<float>													d											= {B.x - A.x, ::fabs(B.y - A.y)};
		float																		error										= d.x / 2.0f;
		const int32_t																ystep										= (A.y < B.y) ? 1 : -1;
		int32_t																		y											= (int32_t)A.y;
		int32_t																		pixelsDrawn									= 0;
		if(steep) {
			for(int32_t x = (int32_t)A.x, xStop = (int32_t)B.x; x < xStop; ++x) {
				if(::gpk::in_range(x, 0, (int32_t)bitmapTarget.metrics().y) && ::gpk::in_range(y, 0, (int32_t)bitmapTarget.metrics().x)) {
					callback(bitmapTarget.begin(), bitmapTarget.metrics(), {(uint32_t)x, (uint32_t)y}, &value);
					++pixelsDrawn;
				}
				error																	-= d.y;
				if(error < 0) {
					y																		+= ystep;
					error																	+= d.x;
				}
			}
		}
		else {
			for(int32_t x = (int32_t)A.x, xStop = (int32_t)B.x; x < xStop; ++x) {
				if(::gpk::in_range(y, 0, (int32_t)bitmapTarget.metrics().y) && ::gpk::in_range(x, 0, (int32_t)bitmapTarget.metrics().x)) {
					callback(bitmapTarget.begin(), bitmapTarget.metrics(), {(uint32_t)x, (uint32_t)y}, &value);
					++pixelsDrawn;
				}
				error																	-= d.y;
				if(error < 0) {
					y																		+= ystep;
					error																	+= d.x;
				}
			}
		}
		return pixelsDrawn;
	}

	// Bresenham's line algorithm
	template<typename _tCoord, typename _tColor>
	static					::gpk::error_t									drawLine									(::gpk::view_grid<_tColor>& bitmapTarget, const _tColor& value, const ::gpk::SLine2D<_tCoord>& line)				{
		::gpk::SCoord2<float>														A											= line.A.Cast<float>();
		::gpk::SCoord2<float>														B											= line.B.Cast<float>();
		const bool																	steep										= (::fabs(B.y - A.y) > ::fabs(B.x - A.x));
		if(steep){
			::std::swap(A.x, A.y);
			::std::swap(B.x, B.y);
		}
		if(A.x > B.x) {
			::std::swap(A.x, B.x);
			::std::swap(A.y, B.y);
		}
		const ::gpk::SCoord2<float>													d											= {B.x - A.x, ::fabs(B.y - A.y)};
		float																		error										= d.x / 2.0f;
		const int32_t																ystep										= (A.y < B.y) ? 1 : -1;
		int32_t																		y											= (int32_t)A.y;
		int32_t																		pixelsDrawn									= 0;
		if(steep) {
			for(int32_t x = (int32_t)A.x, xStop = (int32_t)B.x; x < xStop; ++x) {
				if(::gpk::in_range(x, 0, (int32_t)bitmapTarget.metrics().y) && ::gpk::in_range(y, 0, (int32_t)bitmapTarget.metrics().x)) {
					bitmapTarget[x][y]														= value;
					++pixelsDrawn;
				}
				error																	-= d.y;
				if(error < 0) {
					y																		+= ystep;
					error																	+= d.x;
				}
			}
		}
		else {
			for(int32_t x = (int32_t)A.x, xStop = (int32_t)B.x; x < xStop; ++x) {
				if(::gpk::in_range(y, 0, (int32_t)bitmapTarget.metrics().y) && ::gpk::in_range(x, 0, (int32_t)bitmapTarget.metrics().x)) {
					bitmapTarget[y][x]														= value;
					++pixelsDrawn;
				}
				error																	-= d.y;
				if(error < 0) {
					y																		+= ystep;
					error																	+= d.x;
				}
			}
		}
		return pixelsDrawn;
	}

	// Bresenham's line algorithm
	template<typename _tCoord>
	static					::gpk::error_t									drawLine									(const ::gpk::SCoord2<uint32_t>& targetMetrics, const ::gpk::SLine2D<_tCoord>& line, ::gpk::array_pod<::gpk::SCoord2<int32_t>>& out_Points)				{
		::gpk::SCoord2<float>														A											= line.A.Cast<float>();
		::gpk::SCoord2<float>														B											= line.B.Cast<float>();
		const bool																	steep										= (::fabs(B.y - A.y) > ::fabs(B.x - A.x));
		if(steep){
			::std::swap(A.x, A.y);
			::std::swap(B.x, B.y);
		}
		if(A.x > B.x) {
			::std::swap(A.x, B.x);
			::std::swap(A.y, B.y);
		}
		const ::gpk::SCoord2<float>													d											= {B.x - A.x, ::fabs(B.y - A.y)};
		float																		error										= d.x / 2.0f;
		const int32_t																ystep										= (A.y < B.y) ? 1 : -1;
		int32_t																		y											= (int32_t)A.y;
		int32_t																		pixelsDrawn									= 0;
		if(steep) {
			for(int32_t x = (int32_t)A.x, xStop = (int32_t)B.x; x < xStop; ++x) {
				if(::gpk::in_range(x, 0, (int32_t)targetMetrics.y) && ::gpk::in_range(y, 0, (int32_t)targetMetrics.x)) {
					out_Points.push_back({y, x});
					++pixelsDrawn;
				}
				error																	-= d.y;
				if(error < 0) {
					y																		+= ystep;
					error																	+= d.x;
				}
			}
		}
		else {
			for(int32_t x = (int32_t)A.x, xStop = (int32_t)B.x; x < xStop; ++x) {
				if(::gpk::in_range(y, 0, (int32_t)targetMetrics.y) && ::gpk::in_range(x, 0, (int32_t)targetMetrics.x)) {
					out_Points.push_back({x, y});
					++pixelsDrawn;
				}
				error																	-= d.y;
				if(error < 0) {
					y																		+= ystep;
					error																	+= d.x;
				}
			}
		}
		return pixelsDrawn;
	}


	// Bresenham's line algorithm
	template<typename _tCoord>
	static					::gpk::error_t									drawLine									(const ::gpk::SCoord2<uint32_t>& targetMetrics, const ::gpk::SLine3D<_tCoord>& line, ::gpk::array_pod<::gpk::SCoord2<int32_t>>& out_Points)				{
		return drawLine(targetMetrics, ::gpk::SLine2D<_tCoord>{{line.A.x, line.A.y}, {line.B.x, line.B.y}}, out_Points);
	}
} // namespace

#endif // BITMAP_TARGET_H_98237498023745654654
