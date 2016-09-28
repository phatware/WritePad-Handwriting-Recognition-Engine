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

#include <windows.h>
#include "bastypes.h"

#include "ams_mg.h"
#include "hwr_sys.h"
#include "lowlevel.h"

#include "def.h"

#include "vxmain.h"
#include "vx_def.h"
#include "vx_fnc.h"
#include "vx_utl.h"
#include "vx_img.h"
#include "vx_dbg.h"

#include "pg_debug.h"
#include "wlink.h"

static _SHORT LineCompare(p_SHORT x, p_SHORT y, _SHORT N, _SHORT I0);

#define               MAX_ELEM  XRINP_SIZE

static  p_xrdata_type p_xrdata;
static  p_xrd_el_type p_xrd;

static  AEXTR         Elem[MAX_ELEM];

static _SHORT         nElem = 0;

#define KO            12
#define KY             7

#define DX            64
#define DY            64

_SHORT    WriteTapData(char *name, p_SHORT pX, p_SHORT pY, _SHORT ii);

static _SHORT  nNum, nTrc, nDsc;

static _BOOL bXrElem;

_VOID SetXrElemPtr(p_xrdata_type pPtr)
{
	p_xrdata = pPtr;
	p_xrd = (p_xrd_el_type) (pPtr->xrd);
}

p_xrd_el_type GetXrElemPtr()
{
	if (!bXrElem)
	{
		return(_NULL);
	}
	return(p_xrd);
}

_SHORT GetXrElemNum()
{
	if (!bXrElem)
	{
		return(0);
	}
	return(nElem);
}

_VOID DelXrElemAll()
{
	bXrElem = _FALSE;
}

_BOOL ElemAdd(_SHORT Type, p_SHORT x, p_SHORT y, _SHORT i1, _SHORT i0, _SHORT i2, _SHORT j1, _SHORT j0, _SHORT j2, _SHORT nT)
{
	char    szBuff[128];
	_SHORT  AngA, DirA;
	p_SHORT xx;
	p_SHORT yy;
	_BOOL   bRet = _TRUE;
	_LONG   S = (i2 - i1 + 1)*sizeof(_SHORT);
	_SHORT  i, N, i0_new;
	_RECT   rc;

	if (Type == NONE)
	{
		bXrElem = _TRUE;
		nElem = 0;
		GetTraceBox(x, y, i1, i2, (p_RECT) &rc);
		nTrc = (_SHORT) SD_Create("XR-Elements(Real)");
		SD_Select(nTrc, 2);
		SD_Window((short) rc.left, (short) rc.top, (short) (rc.right - rc.left), (short) (rc.bottom - rc.top));
		nDsc = (_SHORT) SD_Create("XR-Elements(Discret)");
		SD_Select(nDsc, 3);
		SD_Window((short) rc.left, (short) rc.top, (short) (rc.right - rc.left), (short) (rc.bottom - rc.top));
		return(_TRUE);
	}
	if (nElem >= MAX_ELEM)
	{
		return(_FALSE);
	}
	if (Type == TEAR)
	{
		xrinp_type *xrbend = &(p_xrd[nElem].xr);

		Elem[nElem].xx = nElem ? Elem[nElem - 1].xx : 0;

		xrbend->type = TEAR_X;
		xrbend->depth = 0;
		xrbend->shift = 0;
		xrbend->height = 8;
		xrbend->orient = 22;   // == 270 deg
		xrbend->attrib = (_CHAR) nT;   // Tail
		p_xrd[nElem].begpoint = 0;
		p_xrd[nElem].endpoint = 0;
		nElem++;
		p_xrdata->len = nElem;
		return(_TRUE);
	}

	if ((xx = (p_SHORT) HWRMemoryAlloc(S)) == _NULL)
	{
		bRet = _FALSE;
	}
	if ((yy = (p_SHORT) HWRMemoryAlloc(S)) == _NULL)
	{
		bRet = _FALSE;
	}

	if (!bRet)
	{
		if (xx != _NULL)
		{
			HWRMemoryFree(xx);
		}
		if (yy != _NULL)
		{
			HWRMemoryFree(yy);
		}
		return(_FALSE);
	}
	AngA = CalcAng12(x[i0], y[i0], x[i1], y[i1], x[i2], y[i2]);
	DirA = CalcDir12(x[i0], y[i0], x[i1], y[i1], x[i2], y[i2]);

	Elem[nElem].xx = x[i0];
	Elem[nElem].yy = y[i0];
	Elem[nElem].i0 = i0;
	Elem[nElem].i1 = i1;
	Elem[nElem].i2 = i2;
	Elem[nElem].j0 = j0;
	Elem[nElem].j1 = j1;
	Elem[nElem].j2 = j2;

	for (i0_new = -1, N = 0, i = i1; i <= i2; i++)   // copy & turn
	{
		if (y[i] == BREAK)
		{
			continue;
		}
		xx[N] = (_SHORT) (x[i0] + (x[i] - x[i0])*HCos((_SHORT) (-90 - DirA)) - (y[i] - y[i0])*HSin((_SHORT) (-90 - DirA)));
		yy[N] = (_SHORT) (y[i0] + (x[i] - x[i0])*HSin((_SHORT) (-90 - DirA)) + (y[i] - y[i0])*HCos((_SHORT) (-90 - DirA)));
		if (i == i0)
		{
			i0_new = N;
		}
		N++;
	}
	i1 = 0;
	i0 = i0_new;
	i2 = N - 1;
#if PG_DEBUG
	if (mpr == 39)
	{
		WriteTapData("c:\\temp\\dump.tap", xx, yy, N);
	}
#endif
	GetTraceBox(xx, yy, i1, i2, (p_RECT) &rc);
	for (i = 0; i < N; i++)
	{
		xx[i] -= rc.left;
		yy[i] -= rc.top;
	}
	GetTraceBox(xx, yy, (_SHORT) 0, (_SHORT) (N - 1), (p_RECT) &rc);
	for (i = 0; i < N; i++)
	{
		xx[i] = (_SHORT) ((xx[i])*(((_DOUBLE) DX*2.0) / ((_DOUBLE) rc.right + 1.0)));
		yy[i] = (_SHORT) ((yy[i])*(((_DOUBLE) DY*1.0) / ((_DOUBLE) rc.bottom + 1.0)));
	}
	Elem[nElem].Ornt = DirA;
	Elem[nElem].dx = rc.right + 1;
	Elem[nElem].dy = rc.bottom + 1;
	if (Type == EXTR)
	{
		Elem[nElem].bRev = _FALSE;
	}
	else
		if (Type == -EXTR)
		{
			Elem[nElem].bRev = _TRUE;
		}

	GetTraceBox(xx, yy, (_SHORT) 0, (_SHORT) (N - 1), (p_RECT) &rc);
	// Type ********
	Elem[nElem].Type = (_CHAR) (AngA < 1 ? 0 : LineCompare(xx, yy, N, i0));
	// Discret *****
	{
		xrinp_type *xrbend = &(p_xrd[nElem].xr);

		Elem[nElem].Wide = VxSetWide(Elem[nElem].dx, Elem[nElem].dy);
		if (Elem[nElem].Wide == 0)
		{
			Elem[nElem].Type = 1;
		}

		xrbend->type = Elem[nElem].bRev * 32 + Elem[nElem].Type * 8 + Elem[nElem].Wide;
		xrbend->depth = (_UCHAR) VxSetSize(Elem[nElem].dy);
		xrbend->shift = (_UCHAR) VxSetPosX((_SHORT) (nElem ? Elem[nElem].xx - Elem[nElem - 1].xx : 0));
		xrbend->height = (_UCHAR) VxSetPosY(Elem[nElem].yy);
		xrbend->orient = (_UCHAR) VxSetOrnt(Elem[nElem].Ornt);
		xrbend->attrib = (_UCHAR) nT;    // Tail
		xrbend->penalty = (_UCHAR) xrbend->depth;
		p_xrd[nElem].begpoint = Elem[nElem].j1;
		p_xrd[nElem].endpoint = Elem[nElem].j2;
#if PG_DEBUG
		if (mpr == 30 || mpr == 32)
		{
			wsprintf(szBuff, "%2d)%02X %02d-%03d %2d @%02d", nElem + 1, xrbend->type, xrbend->shift, xrbend->height, xrbend->depth, xrbend->orient*KO + KO / 2);
			DrawElem(Elem[nElem].Type, szBuff, xx, yy, N);
		}
		//        if(mpr==33)
		{
			wsprintf(szBuff, "%02d) %1d:%1d:%1d  %1X:%1X  %1X  @%02X", nElem + 1, GetTypeR(xrbend->type), GetTypeT(xrbend->type), GetTypeW(xrbend->type), xrbend->shift, xrbend->height, xrbend->depth, xrbend->orient);
			DrawElem(Elem[nElem].Type, szBuff, xx, yy, N);
		}
#endif
	}
	if (xx != _NULL)
	{
		HWRMemoryFree(xx);
	}
	if (yy != _NULL)
	{
		HWRMemoryFree(yy);
	}

	nElem++;
	p_xrdata->len = nElem;

	return _TRUE;
}

_VOID DrawAll(p_low_type low_data, p_SHORT x, p_SHORT y, _SHORT n)
{
	p_SHORT xBuf = low_data->xBuf;
	p_SHORT yBuf = low_data->yBuf;
	_SHORT  nReal = low_data->rc->ii;
	_SHORT  i;
	_SHORT  dx = Elem[0].xx;
	_RECT   rect;
	_SHORT  nWndR, nWndT;

	GetTraceBox(xBuf, yBuf, (_SHORT) 0, (_SHORT) (nReal - 1), (p_RECT) &rect);
	nWndR = (_SHORT) SD_Create("Trace(Real)");
	SD_Select(nWndR, 0);
	SD_Window((_SHORT) rect.left, (_SHORT) rect.top, (_SHORT) (rect.right - rect.left), (_SHORT) (rect.bottom - rect.top));
	DrawTrace(xBuf, yBuf, (_SHORT) 0, (_SHORT) (nReal - 1));

	GetTraceBox(x, y, (_SHORT) 0, (_SHORT) (n - 1), (p_RECT) &rect);
	nWndT = (_SHORT) SD_Create("Trace(Scaled)");
	SD_Select(nWndT, 0);
	SD_Window((_SHORT) rect.left, (_SHORT) rect.top, (_SHORT) (rect.right - rect.left), (_SHORT) (rect.bottom - rect.top));
	DrawTrace(x, y, (_SHORT) 0, (_SHORT) (n - 1));

	for (i = 0; i < nElem; i++) if (p_xrd[i].xr.type != TEAR_X)
		{
			SD_Select((_SHORT) nWndT, (_SHORT) (2 + Elem[i].Type));
			DrawTrace(x, y, Elem[i].i1, Elem[i].i2);
			SD_Select(nWndT, 1);
			SD_MoveTo(x[Elem[i].i0] - 2, y[Elem[i].i0] - 2);
			SD_LineTo(x[Elem[i].i0] + 2, y[Elem[i].i0] + 2);
			SD_MoveTo(x[Elem[i].i0] + 2, y[Elem[i].i0] - 2);
			SD_LineTo(x[Elem[i].i0] - 2, y[Elem[i].i0] + 2);

			SD_Select(nWndR, 2 + Elem[i].Type);
			DrawTrace(xBuf, yBuf, Elem[i].j1, Elem[i].j2);
			SD_Select(nWndR, 1);
			SD_MoveTo(xBuf[Elem[i].j0] - 20, yBuf[Elem[i].j0] - 20);
			SD_LineTo(xBuf[Elem[i].j0] + 20, yBuf[Elem[i].j0] + 20);
			SD_MoveTo(xBuf[Elem[i].j0] + 20, yBuf[Elem[i].j0] - 20);
			SD_LineTo(xBuf[Elem[i].j0] - 20, yBuf[Elem[i].j0] + 20);
		}

	for (i = 0; i < nElem; i++)
	{
		xrinp_type *xrbend = &(p_xrd[i].xr);
		_SHORT       bRev = (xrbend->type & 32) > 0;

		if (xrbend->type != TEAR_X)
		{
			dx += VxGetPosX(xrbend->shift);
			DrawLine(nDsc, (_SHORT) (i > 0 ? bRev : -1), (xrbend->type & 24) / 8, dx, VxGetPosY(xrbend->height), VxGetWide((_SHORT) (xrbend->type & 7), VxGetSize(xrbend->depth)), VxGetSize((_SHORT) xrbend->depth), VxGetOrnt((_SHORT) xrbend->orient), 3);
			DrawLine(nTrc, (_SHORT) (i > 0 ? bRev : -1), Elem[i].Type, Elem[i].xx, Elem[i].yy, Elem[i].dx, Elem[i].dy, Elem[i].Ornt, 2);
		}
		DrawXr(i, xrbend);
	}
	SD_Update(-1);
}

_SHORT LineCompare(p_SHORT x, p_SHORT y, _SHORT N, _SHORT I0)
{
	_LONG  lParm = 1000000L;
	LINE  *Line;
	int    nLine, iLine, nPnt, nMid;

	for (nLine = 0, iLine = 0; (Line = ImgGetSample(iLine, &nPnt, &nMid)); iLine++)
	{
		_SHORT  i, j;
		_LONG   lDist, l1, l2, ll;
		_BOOL   bFlg, bRev = x[0] > x[N - 1];

		Line++;
		lDist = 0L;
#if PG_DEBUG
		if (mpr == 30)
		{
			int num = SD_Create("points");

			SD_Window(0, 0, DX * 3, DY * 2);
			SD_Select(num, 1);
		}
#endif
		for (i = 0; i < N; i++)
		{
			if (i == I0)
			{
				continue;
			}

			l1 = CalcLngSq(x[i], y[i], x[I0], y[I0]);

			bFlg = i<I0 && !bRev || i>I0 && bRev;

			for (j = nMid + (bFlg ? -1 : 1); j >= 0 && j < nPnt; j += (bFlg ? -1 : 1))
			{
				if (j == nMid)
				{
					continue;
				}

				l2 = CalcLngSq(Line[j].x, Line[j].y, Line[nMid].x, Line[nMid].y);
				if (l2 >= l1)
				{
					break;
				}
			}
			if (j < 0)
			{
				j = 0;
			}
			if (j >= nPnt)
			{
				j = nPnt - 1;
			}
			ll = CalcLngSq(x[i], y[i], Line[j].x, Line[j].y);
#if PG_DEBUG
			if (mpr == 30)
			{
				SD_MoveTo(DX / 2 + x[i], DY / 2 + y[i]);
				SD_LineTo(DX / 2 + Line[j].x, DY / 2 + Line[j].y);
			}
#endif
			lDist += ll;
		}
		if (lDist < lParm)
		{
			lParm = lDist;
			nLine = iLine;
		}
	}
	return(nLine);
}
