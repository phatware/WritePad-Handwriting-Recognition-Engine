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

#include "bastypes.h"

#include "ams_mg.h"
#include "hwr_sys.h"
#include "lowlevel.h"

#include "vxmain.h"
#include "vx_def.h"
#include "vx_img.h"
#include "vx_fnc.h"
#include "vx_dbg.h"

#include "pg_debug.h"
#include "wlink.h"

#define NCOLOR      4
static  _SHORT Color[NCOLOR] =
{
	EGA_YELLOW,
	EGA_BLUE,
	EGA_LIGHTGREEN,
	EGA_LIGHTCYAN
};

_VOID ShowReady(p_SHORT pReady, p_SHORT X1, p_SHORT X2, p_SHORT x, p_SHORT y, _SHORT n)
{
	_SHORT     i, k;
	_RECT      rc;
	_BOOL      bFirst = _TRUE;

	GetTraceBox(x, y, (_SHORT) 0, (_SHORT) (n - 1), (p_RECT) &rc);

	for (i = 0; i < n; i++)
	{
		k = pReady[i];

		if (k == NONE)
		{
			continue;
		}
		// BLOCK *****
		{
			_RECT  rect;

			rect.left = x[i] - 2;
			rect.top = y[i] - 2;
			rect.right = x[i] + 2;
			rect.bottom = y[i] + 2;
#if PG_DEBUG
			if (k == EXTR)
			{
				draw_line(rect.left, rect.top, rect.right, rect.bottom, EGA_LIGHTGREEN, SOLID_LINE, THICK_WIDTH);
				draw_line(rect.right, rect.top, rect.left, rect.bottom, EGA_LIGHTGREEN, SOLID_LINE, THICK_WIDTH);

				draw_line(x[i], y[i], x[X1[i]], y[X1[i]], EGA_LIGHTGREEN, SOLID_LINE, NORM_WIDTH);
				draw_line(x[i], y[i], x[X2[i]], y[X2[i]], EGA_LIGHTGREEN, SOLID_LINE, NORM_WIDTH);
			}
			if (k == ZONE)
			{
				draw_line(rect.left, rect.top, rect.right, rect.bottom, EGA_LIGHTCYAN, SOLID_LINE, THICK_WIDTH);
				draw_line(rect.right, rect.top, rect.left, rect.bottom, EGA_LIGHTCYAN, SOLID_LINE, THICK_WIDTH);
			}
#endif // PG_DEBUG
		}
	}
	SD_Update(-1);
}

void ShowAngls(p_SHORT Ready, p_SHORT Direc, p_SHORT Angl1, p_SHORT Angl2, p_SHORT Angl3, p_SHORT AnglT, _SHORT N)
{
	int nNum = (int) SD_Create("Angles");
	int i, j, dy, DX = N, DY = 180 * 5;

	SD_Window(-1, -1, DX, DY);

	for (j = 4; j >= 0; j--)
	{
		SD_Select(nNum, 0);
		SD_MoveTo(N, j * 180 + 180);
		SD_LineTo(0, j * 180 + 180);
		for (i = 0; i < N; i++)
		{
			if (j == 0)
			{
				dy = Angl3[i];
			}
			if (j == 1)
			{
				dy = Angl2[i];
			}
			if (j == 2)
			{
				dy = Angl1[i];
			}
			if (j == 3)
			{
				dy = AnglT[i];
			}
			if (j == 4)
			{
				dy = Direc[i] / 2;
			}

			//if(y[i]==BREAK)     continue;
			if (j == 4)
			{
				SD_Select(nNum, 4);
			}
			else
			{
				SD_Select(nNum, 5);
			}

			SD_LineTo(i, j * 180 + 180 - dy);
		}
	}
	nNum = (int) SD_Create("Extremes");
	SD_Window(-1, -1, DX, DY);
	for (i = 0; i < N; i++)
	{
		if (Ready[i] == TEAR)
		{
			SD_Select(nNum, 0);
		}
		else
			if (Ready[i] == EXTR)
			{
				SD_Select(nNum, 3);
			}
			else
			{
				continue;
			}

		SD_MoveTo(i, 0);
		SD_LineTo(i, DY);
	}
	SD_Update(-1);
}

_VOID DrawLine(_SHORT n, _SHORT nRev, int nLine, int x, int y, int dx, int dy, int DirA, int c)
{
	p_SHORT xx;
	p_SHORT yy;
	_LONG   S;
	_SHORT  i;
	int     nPnt;
	int     nMid;
	LINE   *Line = ImgGetSample(nLine, &nPnt, &nMid);
	_BOOL   bRet = _TRUE;
	_DOUBLE kx, ky;

	S = nPnt*sizeof(_SHORT);

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
		return;
	}
	kx = (_DOUBLE) dx / (IMG_DX*2.0);
	ky = (_DOUBLE) dy / (IMG_DY*1.0);
	for (i = 0; i < nPnt; i++)   // copy & turn
	{
		xx[i] = (_SHORT) (kx*(Line[i].x - Line[nMid].x)*HCos((_SHORT) (90 + DirA)) - ky*(Line[i].y - Line[nMid].y)*HSin((_SHORT) (90 + DirA)));
		yy[i] = (_SHORT) (kx*(Line[i].x - Line[nMid].x)*HSin((_SHORT) (90 + DirA)) + ky*(Line[i].y - Line[nMid].y)*HCos((_SHORT) (90 + DirA)));
	}
	for (i = 0; i < nPnt; i++)   // shift
	{
		xx[i] = xx[i] + x;
		yy[i] = yy[i] + y;
	}

	SD_Select(n, 7);
	if (nRev == 0)
	{
		SD_LineTo(xx[0], yy[0]);
	}
	if (nRev == 1)
	{
		SD_LineTo(xx[nPnt - 1], yy[nPnt - 1]);
	}
	SD_Select(n, c);
	for (i = 0; i < nPnt; i++)
	{
		if (i == 0)
		{
			SD_MoveTo(xx[i], yy[i]);
		}
		else
		{
			SD_LineTo(xx[i], yy[i]);
		}
	}
	if (nRev == 1)
	{
		SD_MoveTo(xx[0], yy[0]);
	}

	if (xx != _NULL)
	{
		HWRMemoryFree(xx);
	}
	if (yy != _NULL)
	{
		HWRMemoryFree(yy);
	}
}

_VOID DrawElem(_SHORT nLine, char *szBuff, p_SHORT xx, p_SHORT yy, _SHORT N)
{
	_SHORT i, j;
	_SHORT nNum = (_SHORT) SD_Create(szBuff);
	LINE  *Line;
	int    nPnt, nMid;

	SD_Window(0, 0, IMG_DX * 3, IMG_DY * 2);
	SD_Select(nNum, 3);

	for (i = 0; i < N; i++)
	{
		if (i == 0)
		{
			SD_MoveTo(IMG_DX / 2 + xx[i], IMG_DY / 2 + yy[i]);
		}
		else
		{
			SD_LineTo(IMG_DX / 2 + xx[i], IMG_DY / 2 + yy[i]);
		}
	}
	for (j = 0; (Line = ImgGetSample(j, &nPnt, &nMid)) != _NULL; j++) for (i = 0; i < nPnt; i++)
		{
			if (j == nLine)
			{
				SD_Select(nNum, 2);
			}
			else
			{
				SD_Select(nNum, 7);
			}
			if (i == 0)
			{
				SD_MoveTo(IMG_DX / 2 + Line[i].x, IMG_DY / 2 + Line[i].y);
			}
			else
			{
				SD_LineTo(IMG_DX / 2 + Line[i].x, IMG_DY / 2 + Line[i].y);
			}
		}
	SD_Select(nNum, 1);
	{
		SD_MoveTo(IMG_DX / 2 + 0, IMG_DY / 2 + 0);
		SD_LineTo(IMG_DX / 2 + 2 * IMG_DX, IMG_DY / 2 + 0);
		SD_LineTo(IMG_DX / 2 + 2 * IMG_DX, IMG_DY / 2 + IMG_DY);
		SD_LineTo(IMG_DX / 2 + 0, IMG_DY / 2 + IMG_DY);
		SD_LineTo(IMG_DX / 2 + 0, IMG_DY / 2 + 0);

		SD_MoveTo(IMG_DX / 2 + IMG_DX, IMG_DY / 2 + 0);
		SD_LineTo(IMG_DX / 2 + IMG_DX, IMG_DY / 2 + IMG_DY);
	}
}


_VOID DrawXr(_SHORT n, xrinp_type *xrbend)
{
	static    _SHORT nNum;
	XR_IMAGE         XrImage;

	if (n == 0)
	{
		nNum = SD_Create("Xr");
	}
	XrImage.type = xrbend->type;
	XrImage.size = xrbend->depth;
	XrImage.shift = xrbend->shift;
	XrImage.height = xrbend->height;
	XrImage.orient = xrbend->orient;
	XrImage.penalty = xrbend->penalty;
	XrImage.attrib = xrbend->attrib;
	XrImage.emp = xrbend->emp;

	SD_Select(nNum, 0);
	SD_SendDT(SD_TYPE_IMAGE, (void*) &XrImage, sizeof(XrImage));
}


_VOID DrawTrace(p_SHORT x, p_SHORT y, _SHORT i1, _SHORT i2)
{
	_BOOL  bFirst;
	_SHORT i;

	for (bFirst = _TRUE, i = i1; i <= i2; i++)
	{
		if (y[i] == -1)
		{
			bFirst = _TRUE;
		}
		else
		{
			if (bFirst)
			{
				SD_MoveTo(x[i], y[i]);
				bFirst = _FALSE;
			}
			else
			{
				SD_LineTo(x[i], y[i]);
			}
		}
	}
}
