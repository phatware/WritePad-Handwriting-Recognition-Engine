/***************************************************************************************
 *
 *  WRITEPAD(r): Handwriting Recognition Engine (HWRE) and components.
 *  Copyright (c) 2001-2016 PhatWare (r) Corp. All rights reserved.
 *
 *  Licensing and other inquires: <developer@phatware.com>
 *  Developer: Stan Miasnikov, et al. (c) PhatWare Corp. <http://www.phatware.com>
 *
 *  WRITEPAD HWRE is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  THE MATERIAL EMBODIED ON THIS SOFTWARE IS PROVIDED TO YOU "AS-IS"
 *  AND WITHOUT WARRANTY OF ANY KIND, EXPRESS, IMPLIED OR OTHERWISE,
 *  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY OR
 *  FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL PHATWARE CORP.
 *  BE LIABLE TO YOU OR ANYONE ELSE FOR ANY DIRECT, SPECIAL, INCIDENTAL,
 *  INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES WHATSOEVER,
 *  INCLUDING WITHOUT LIMITATION, LOSS OF PROFIT, LOSS OF USE, SAVINGS
 *  OR REVENUE, OR THE CLAIMS OF THIRD PARTIES, WHETHER OR NOT PHATWARE CORP.
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH LOSS, HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE
 *  POSSESSION, USE OR PERFORMANCE OF THIS SOFTWARE.
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with WritePad.  If not, see <http://www.gnu.org/licenses/>.
 *
 **************************************************************************************/

#define STRICT
#define _REQ_WIN
#include <windows.h>
#include <windowsx.h>
#ifdef _PENWIN
#include <penwin.h>
#else
#include "pensub.h32"
#endif
#include <memory.h>
#include <grlib.h>
#include <bastypes.h>
#include <wg_stuff.h>
#include "wggbl.h"
#include "wgprf.h"
#include "wgtrc.h"
#include "wgrec.h"
#include "wgxrd.h"

static HFONT hFontXr = NULL;
static int   fontSize = 0;

BYTE  xr_colors[32] = { 7, 15, 7, 9, 1, 13, 5, 11, 2, 10, 6, 14, 4, 12, 15, 15,
                        15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15
                      };
BYTE  xr_heights[32] = { 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 7, 2,
                         10, 9, 4, 1, 11, 7, 1, 7, 7, 7, 7, 7, 7, 7, 7, 7
                       };

COLORREF ColorAndHeight [] =
{
	{ RGB(255, 255, 255) }, { RGB(192, 192, 192) },
	{ RGB(64, 64, 255) }, { RGB(0, 0, 212) },
	{ RGB(255, 0, 255) }, { RGB(128, 0, 128) },
	{ RGB(0, 255, 255) }, { RGB(0, 128, 0) },
	{ RGB(0, 255, 0) }, { RGB(128, 128, 0) },
	{ RGB(255, 255, 0) }, { RGB(128, 0, 0) },
	{ RGB(255, 0, 0) }, { RGB(192, 192, 192) },
	{ RGB(192, 192, 192) }, { RGB(192, 192, 192) },
	{ RGB(192, 192, 192) }, { RGB(192, 192, 192) },
	{ RGB(192, 192, 192) }, { RGB(192, 192, 192) }
};

void ColorAttributes(HDC hDC, int x, int y, p_XR pxr);

/******************************************************************/

void ColorAttributes(HDC hDC, int x, int y, p_XR pxr)
{
	if (pxr->a & WG_OVR_FLAG)
	{
		SetPixel(hDC, x, y - 1, WG_OVR_FLAG_COLOR);
		SetPixel(hDC, x, y, WG_OVR_FLAG_COLOR);
	}
	if (pxr->a & WG_XSTRICT_FLAG)
	{
		SetPixel(hDC, x + 2, y - 1, WG_XSTRICT_FLAG_COLOR);
		SetPixel(hDC, x + 2, y, WG_XSTRICT_FLAG_COLOR);
	}
	if (pxr->a & WG_HSTRICT_FLAG)
	{
		SetPixel(hDC, x + 4, y - 1, WG_HSTRICT_FLAG_COLOR);
		SetPixel(hDC, x + 4, y, WG_HSTRICT_FLAG_COLOR);
	}
	if (pxr->a & WG_TAIL_FLAG)
	{
		SetPixel(hDC, x + 6, y - 1, WG_TAIL_FLAG_COLOR);
		SetPixel(hDC, x + 6, y, WG_TAIL_FLAG_COLOR);
	}
} /* end of ColorAttributes */

/******************************************************************/
DWORD FAR xrdDraw(HDC hDC, int x, int y, int size, LPSTR pxr, int inc,
                  BOOL DrawCenter, LPRECT prc)
{
	char            string[1];
	int             color;
	HFONT           hFontOld;
	LOGFONT         lf;
	TEXTMETRIC      tm;
	COLORREF        clxr;
	int             h;

	DWORD       widthheight;
#ifdef _PENWIN
	p_XR          p_xr = (p_XR)pxr;
#else
	SIZE          sizept;
	p_LAB_XRDATA  p_xr = (p_LAB_XRDATA) pxr;
#endif

	if (!prfDrawXR())
	{
		return 0L;
	}
#ifdef _PENWIN
	h = p_xr->h;
	string[0] = ' ' + p_xr->xr;
#else
	h = p_xr->xdxr.xnh & 0xF;
	string[0] = ' ' + p_xr->xdxr.xnxr;
#endif
	y -= xr_heights[h] * inc * size / 10;
	color = xr_colors[h];
	{
		if (h > sizeof(ColorAndHeight) / sizeof(COLORREF) || h == 0)
		{
			clxr = RGB(192, 192, 192);
		}
		else
		{
			clxr = ColorAndHeight[h - 1];
		}
		SetTextColor(hDC, clxr);
		SetBkColor(hDC, RGB(0, 0, 0));
	}
	if (hFontXr == NULL || size != fontSize)
	{
		if (hFontXr != NULL)
		{
			DeleteObject(hFontXr);
			hFontXr = NULL;
		}
		_fmemset(&lf, 0, sizeof(LOGFONT));
		lf.lfHeight = size;
		lstrcpy(lf.lfFaceName, "XrPen");
		hFontXr = CreateFontIndirect(&lf);
		fontSize = size;
	}
	hFontOld = (HFONT) SelectObject(hDC, hFontXr);
	GetTextMetrics(hDC, &tm);
#ifdef _PENWIN
	widthheight = GetTextExtent(hDC, string, 1);
#else
	GetTextExtentPoint(hDC, string, 1, &sizept);
	widthheight = MAKELONG(sizept.cx, sizept.cy);
#endif

	if (DrawCenter)
	{
		x = x + (tm.tmMaxCharWidth - LOWORD(widthheight)) / 2;
		widthheight = MAKELONG(tm.tmMaxCharWidth, sizept.cy);
	}
	ExtTextOut(hDC, x, y, 0, NULL, string, 1, NULL);
	if (prc != NULL)
	{
		SetRect(prc, x, y, x + LOWORD(widthheight), y + HIWORD(widthheight));
	}
	ColorAttributes(hDC, x, y, (p_XR) pxr);
	SelectObject(hDC, hFontOld);
	return widthheight;
} /* xrdDraw */

/******************************************************************/
void FAR xrdPut(HDC hDC, int x, int y, p_XR pxr, int inc)
{
	char    string[1];
	int     color;
	char    r, g, b, s;

	//  y -= xr_heights[pxr->h] * inc;
	if (!prfDrawXR())
	{
		return;
	}
	color = xr_colors[pxr->h & 0xF];
	b = (char) ((color & 0x0001) ? 127 : 0);
	g = (char) ((color & 0x0002) ? 127 : 0);
	r = (char) ((color & 0x0004) ? 127 : 0);
	s = (char) ((color & 0x0008) ? 2 : 1);
	SetTextColor(hDC, RGB(r*s, g*s, b*s));
	SetBkColor(hDC, RGB(0, 0, 0));
	string[0] = ' ' + pxr->xr;
	ExtTextOut(hDC, x, y, 0, NULL, string, 1, NULL);
	ColorAttributes(hDC, x, y, pxr);
} /* end of xrdPut */

/******************************************************************/
void FAR xrdDone(void)
{
	if (hFontXr != NULL)
	{
		DeleteObject(hFontXr);
		hFontXr = NULL;
	}
} /* xrdDone */
