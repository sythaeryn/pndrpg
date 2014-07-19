// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "stdmisc.h"

#include "nel/misc/cursor.h"
#include "nel/misc/system.h"

namespace NLMISC {

float CCursor::_CursorScale = 0.f;

CCursor::CCursor(CWindow *window, ICursor *native):_Window(window), _Native(native),
	ColorDepth(ColorDepth32), OrigHeight(32), HotspotScale(1.f),
	HotspotOffsetX(0), HotspotOffsetY(0),
	HotSpotX(0), HotSpotY(0), Col(CRGBA::White), Rot(0)
{
}

CCursor::~CCursor()
{
	reset();

	if (_Native)
	{
		delete _Native;
		_Native = NULL;
	}
}

void CCursor::reset()
{
	if (_Native) _Native->reset();
}

void CCursor::setBitmap(const CBitmap &bitmap)
{
	nlassert(bitmap.getWidth() != 0);
	nlassert(bitmap.getHeight() != 0);

	// find used part base on alpha, to avoid too much shrinking
	const CRGBA *pixels = (const CRGBA *) &bitmap.getPixels()[0];
	uint minX, maxX, minY, maxY;
	uint width = bitmap.getWidth();
	uint height = bitmap.getHeight();

	minX = 0;
	for (uint x = 0; x < width; ++x)
	{
		bool stop = false;
		minX = x;
		for (uint y = 0; y < height; ++y)
		{
			if(pixels[x + y * width].A != 0)
			{
				stop = true;
				break;
			}
		}
		if (stop) break;
	}

	maxX = width - 1;
	for (sint x = width - 1; x >= 0; --x)
	{
		bool stop = false;
		maxX = (uint) x;
		for (uint y = 0; y < height; ++y)
		{
			if(pixels[x + y * width].A != 0)
			{
				stop = true;
				break;
			}
		}
		if (stop) break;
	}

	minY = 0;
	for (uint y = 0; y < height; ++y)
	{
		bool stop = false;
		minY = y;
		for (uint x = 0; x < width; ++x)
		{
			if(pixels[x + y * width].A != 0)
			{
				stop = true;
				break;
			}
		}
		if (stop) break;
	}

	maxY = height - 1;
	for (sint y = height - 1; y >= 0; --y)
	{
		bool stop = false;
		maxY = (uint) y;
		for (uint x = 0; x < width; ++x)
		{
			if(pixels[x + y * width].A != 0)
			{
				stop = true;
				break;
			}
		}
		if (stop) break;
	}

	uint destWidth = 32, destHeight = 32;
	_Native->getBestSize(width, height, destWidth, destHeight);

	// build a square bitmap
	uint tmpSize = std::max(maxX - minX + 1, maxY - minY + 1);
	Src.resize(tmpSize, tmpSize);
	// blit at top left corner
	Src.blit(bitmap, minX, minY, maxX - minX + 1, maxY - minY + 1, 0, 0);

	OrigHeight = bitmap.getHeight();
	HotspotOffsetX = minX;
	HotspotOffsetY = minY;

	HotspotScale = _CursorScale;
	clamp(HotspotScale, 0.f, 1.f);
	// first resampling, same for all cursors
	tmpSize = (uint) (tmpSize * HotspotScale);
	if (tmpSize == 0) tmpSize = 1;

	if (HotspotScale < 1.f)
	{
		Src.resample(tmpSize, tmpSize);
	}

	// shrink if necessary
	if (tmpSize > destWidth || tmpSize > destHeight) // need to shrink ?
	{
		// constraint proportions
		HotspotScale *= std::min(float(destWidth) / tmpSize, float(destHeight) / tmpSize);
		Src.resample(destWidth, destHeight);
	}
	else
	{
		CBitmap final;
		final.resize(destWidth, destHeight);
		final.blit(&Src, 0, 0);
		Src.swap(final);
	}
}

void CCursor::setCursorScale(float scale)
{
	_CursorScale = scale;
}

bool CCursor::buildCursor()
{
	nlassert(CSystem::instance()->isAlphaBlendedCursorSupported());

	uint mouseW = 32, mouseH = 32;
	_Native->getBestSize(Src.getWidth(), Src.getHeight(), mouseW, mouseH);

	CBitmap rotSrc = Src;
	if (Rot > 3) Rot = 3; // mimic behavior of 'CViewRenderer::drawRotFlipBitmapTiled' (why not rot & 3 ??? ...)
	switch(Rot)
	{
		case 0: break;
		case 1: rotSrc.rot90CW(); break;
		case 2: rotSrc.rot90CW(); rotSrc.rot90CW(); break;
		case 3: rotSrc.rot90CCW(); break;
	}

	// create a native cursor from bitmap
	return _Native->buildCursor(rotSrc, mouseW, mouseH, ColorDepth == ColorDepth16 ? 16:32, Col, HotSpotX, HotSpotY);
}

bool CCursor::isValid() const
{
	return _Native->isValid();
}

ICursor::ICursor()
{
#if defined(NL_OS_UNIX) && !defined(NL_OS_MAC)
	Dpy = NULL;
#endif
}

ICursor::~ICursor()
{
}

/*
CCursor& CCursor::operator= (const CCursor& from)
{
	if (&from == this)
		return *this;
	Src = from.Src; // requires more than a surface copy
	OrigHeight = from.OrigHeight;
	HotspotScale = from.HotspotScale;
	HotspotOffsetX = from.HotspotOffsetX;
	HotspotOffsetY = from.HotspotOffsetY;
	HotSpotX = from.HotSpotX;
	HotSpotY = from.HotSpotY;
//	Cursor = from.Cursor;
	Col = from.Col;
	Rot = from.Rot;
#if defined(NL_OS_UNIX) && !defined(NL_OS_MAC)
	Dpy = from.Dpy;
#endif
	return *this;
}
*/
}
