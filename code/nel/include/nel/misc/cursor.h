#ifndef NL_CURSOR_H
#define NL_CURSOR_H

#include "nel/misc/bitmap.h"
#include "nel/misc/rgba.h"

namespace NLMISC {

enum TColorDepth { ColorDepth16 = 0, ColorDepth32, ColorDepthCount };

class CWindow;
class ICursor;

class CCursor
{
public:
	CCursor(CWindow *window = NULL, ICursor *cursor = NULL);
	virtual ~CCursor();

	CCursor& operator= (const CCursor& from);

	void reset();

	void setBitmap(const CBitmap &bitmap);

	static void setCursorScale(float scale);

	bool buildCursor();
	bool isValid() const;

	ICursor* getNative() { return _Native; }

	NLMISC::CBitmap Src;
	TColorDepth		ColorDepth;
	uint			OrigHeight;
	float			HotspotScale;
	uint			HotspotOffsetX;
	uint			HotspotOffsetY;
	sint			HotSpotX;
	sint			HotSpotY;
	NLMISC::CRGBA	Col;
	uint8			Rot;

protected:
	CWindow* _Window;
	ICursor* _Native;

	static float _CursorScale;
};

class ICursor
{
public:
	ICursor();
	virtual ~ICursor();

	virtual bool isValid() const =0;
	virtual bool getBestSize(uint srcWidth, uint srcHeight, uint &dstWidth, uint &dstHeight) =0;
	virtual bool buildCursor(const NLMISC::CBitmap &bitmap, uint iconWidth, uint iconHeight, uint iconDepth, const NLMISC::CRGBA &col, sint hotSpotX, sint hotSpotY) =0;
	virtual bool reset() =0;
};

}

#endif
