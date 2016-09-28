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
#include "def.h"
#include "vx_def.h"
#include "vx_img.h"

#include "line.inc"

#define DU  ((STR_UP  -LIN_UP  )/ 4)
#define DM  ((STR_DOWN-STR_UP  )/ 6)
#define DD  ((LIN_DOWN-STR_DOWN)/ 4)
#define DU2 ((STR_UP  -LIN_UP  )/ 8)
#define DM2 ((STR_DOWN-STR_UP  )/12)
#define DD2 ((LIN_DOWN-STR_DOWN)/ 8)

#define K_SIZE	 7.00
#define K_ORNT  11.25

// ** Sample Image *************************************************************
LINE *ImgGetSample(int n, int *nPnt, int *iMid)
{
	LINE *Line;

	if (n < 0 || n >= MAX_LINES)
	{
		return (LINE *) 0;
	}

	Line = Lines[n];
	if (nPnt)
	{
		*nPnt = Line[0].x;
	}
	if (iMid)
	{
		*iMid = Line[0].y;
	}

	return(Line + 1);
}

// ** Debug Draw Image *********************************************************
static  int     xSze, ySze;
static  int     xMax, yMax;
static  int     xLim, yLim;
static  char    cColor;
static  char   *lpArray;
static _SHORT   DirA;

static  COLORREF Height [] =
{
	RGB(192, 192, 192),
	RGB(255, 255, 255),
	RGB(0, 0, 128),
	RGB(0, 0, 255),
	RGB(128, 0, 128),
	RGB(255, 0, 255),
	RGB(0, 255, 255),
	RGB(0, 255, 0),
	RGB(0, 128, 0),
	RGB(255, 255, 0),
	RGB(128, 128, 0),
	RGB(255, 0, 0),
	RGB(128, 0, 0),
	RGB(192, 192, 192),
	RGB(192, 192, 192),
	RGB(192, 192, 192),
};

void ImgDrwSample(HDC hDC, XR_IMAGE *XrImage, int xPos, int yPos, int dx, int dy)
{
#define     FIRST_XR   190

	static char szData [] = " ";
	int         ornt = 270 - XrImage->orient * 12 - 6;
	HFONT       hXrFnt = CreateFont((int) (dx), (int) (dy), ornt * 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, (LPSTR)"XrPen");
	HFONT       hXxFnt = (HFONT) SelectObject(hDC, hXrFnt);
	int         px, py;

	px = (int) (dx) / 2;
	py = (int) (dy) / 2;

	xPos = (int) (xPos + px - (px*cos(-(double) ornt / VX_RAD) - py*sin(-(double) ornt / VX_RAD)));
	yPos = (int) (yPos + px - (px*sin(-(double) ornt / VX_RAD) + py*cos(-(double) ornt / VX_RAD)));

	szData[0] = XrImage->type + FIRST_XR;

	cColor = XrImage->height;
	SetTextColor(hDC, Height[cColor & 0xF]);
	SetBkMode(hDC, TRANSPARENT);
	TextOut(hDC, xPos, yPos, szData, 1);

	if (IsTailL(XrImage->attrib))
	{
		szData[0] = (char) (TAIL_L + FIRST_XR);
		TextOut(hDC, xPos, yPos, szData, 1);

	}
	if (IsTailR(XrImage->attrib))
	{
		szData[0] = (char) (TAIL_R + FIRST_XR);
		TextOut(hDC, xPos, yPos, szData, 1);
	}
	SelectObject(hDC, hXxFnt);
	DeleteObject(hXrFnt);
}

_SHORT VxGetSize(_SHORT n)
{
	return (_SHORT) (n*K_SIZE);
}

_SHORT VxSetSize(_SHORT n)
{
	n = (_SHORT) (n - K_SIZE / 2);
	if (n < 0)
	{
		n = 0;
	}

	n = (_SHORT) (n / K_SIZE);
	if (n > 15)
	{
		n = 15;
	}

	return n;
}

_SHORT VxGetOrnt(_SHORT n)
{
	return (_SHORT) (n*K_ORNT);
}

_SHORT VxSetOrnt(_SHORT n)
{
	n = (_SHORT) (n - K_ORNT / 2);
	if (n < 0)
	{
		n = 0;
	}

	return (_SHORT) (n / K_ORNT);
}

_SHORT VxSetWide(_SHORT dx, _SHORT dy)
{
	_DOUBLE wd = (_DOUBLE) dy / (_DOUBLE) dx;

	if (wd < 0.4)
	{
		return(7);    // 0.3
	}
	if (wd < 0.6)
	{
		return(6);    // 0.5
	}
	if (wd < 0.8)
	{
		return(5);    // 0.7
	}
	if (wd < 1.2)
	{
		return(4);    // 1.0
	}
	if (wd < 1.7)
	{
		return(3);    // 1.5
	}
	if (wd < 2.5)
	{
		return(2);    // 2.0
	}
	if (wd < 4.5)
	{
		return(1);    // 3.0
	}
	/* >=4.5*/ return(0); // 4.0
}

_SHORT VxGetWide(_SHORT wd, _SHORT dy)
{
	if (wd == 7)
	{
		return((_SHORT) (dy / 0.3));
	}
	if (wd == 6)
	{
		return((_SHORT) (dy / 0.5));
	}
	if (wd == 5)
	{
		return((_SHORT) (dy / 0.6));
	}
	if (wd == 4)
	{
		return((_SHORT) (dy / 1.0));
	}
	if (wd == 3)
	{
		return((_SHORT) (dy / 1.5));
	}
	if (wd == 2)
	{
		return((_SHORT) (dy / 2.0));
	}
	if (wd == 1)
	{
		return((_SHORT) (dy / 3.0));
	}
	if (wd == 0)
	{
		return((_SHORT) (dy / 9.0));
	}

	return 0;
}

_SHORT VxGetPosX(_SHORT dx)
{
	if (dx == 0)
	{
		return(-90);
	}
	if (dx == 1)
	{
		return(-66);
	}
	if (dx == 2)
	{
		return(-48);
	}
	if (dx == 3)
	{
		return(-33);
	}
	if (dx == 4)
	{
		return(-20);
	}
	if (dx == 5)
	{
		return(-9);
	}
	if (dx == 6)
	{
		return(0);
	}
	if (dx == 7)
	{
		return(8);
	}
	if (dx == 8)
	{
		return(17);
	}
	if (dx == 9)
	{
		return(28);
	}
	if (dx == 10)
	{
		return(41);
	}
	if (dx == 11)
	{
		return(56);
	}
	if (dx == 12)
	{
		return(74);
	}
	if (dx == 13)
	{
		return(96);
	}
	if (dx == 14)
	{
		return(110);
	}
	if (dx == 15)
	{
		return(150);
	}

	return 0;
}

_SHORT VxSetPosX(_SHORT dx)
{
	if (dx < -76)
	{
		return(0);
	}
	if (dx < -56)
	{
		return(1);
	}
	if (dx < -40)
	{
		return(2);
	}
	if (dx < -26)
	{
		return(3);
	}
	if (dx < -14)
	{
		return(4);
	}
	if (dx < -4)
	{
		return(5);
	}
	if (dx < 4)
	{
		return(6);
	}
	if (dx < 12)
	{
		return(7);
	}
	if (dx < 22)
	{
		return(8);
	}
	if (dx < 34)
	{
		return(9);
	}
	if (dx < 48)
	{
		return(10);
	}
	if (dx < 64)
	{
		return(11);
	}
	if (dx < 84)
	{
		return(12);
	}
	if (dx < 108)
	{
		return(13);
	}
	if (dx < 136)
	{
		return(14);
	}
	return(15);
}

_SHORT VxSetPosY(_SHORT y)
{
	if (y < LIN_UP + 0 * DU + DU2)
	{
		return(1);
	}
	if (y < LIN_UP + 1 * DU + DU2)
	{
		return(2);
	}
	if (y < LIN_UP + 2 * DU + DU2)
	{
		return(3);
	}
	if (y < LIN_UP + 3 * DU + DU2)
	{
		return(4);
	}
	if (y < STR_UP + 0 * DM + DM2)
	{
		return(5);
	}
	if (y < STR_UP + 1 * DM + DM2)
	{
		return(6);
	}
	if (y < STR_UP + 2 * DM + DM2)
	{
		return(7);
	}
	if (y < STR_UP + 3 * DM + DM2)
	{
		return(8);
	}
	if (y < STR_UP + 4 * DM + DM2)
	{
		return(9);
	}
	if (y < STR_UP + 5 * DM + DM2)
	{
		return(10);
	}
	if (y < STR_UP + 6 * DM + DD2)
	{
		return(11);
	}
	if (y < LIN_DOWN - 3 * DD + DD2)
	{
		return(12);
	}
	if (y < LIN_DOWN - 2 * DD + DD2)
	{
		return(13);
	}
	if (y < LIN_DOWN - 1 * DD + DD2)
	{
		return(14);
	}
	return(15);
}

_SHORT VxGetPosY(_SHORT n)
{
	if (n == 1)
	{
		return(LIN_UP + 0 * DU);    // LIN
	}
	if (n == 2)
	{
		return(LIN_UP + 1 * DU);
	}
	if (n == 3)
	{
		return(LIN_UP + 2 * DU);
	}
	if (n == 4)
	{
		return(LIN_UP + 3 * DU);
	}
	if (n == 5)
	{
		return(STR_UP + 0 * DM);    // STR
	}
	if (n == 6)
	{
		return(STR_UP + 1 * DM);
	}
	if (n == 7)
	{
		return(STR_UP + 2 * DM);
	}
	if (n == 8)
	{
		return(STR_UP + 3 * DM);    // MID
	}
	if (n == 9)
	{
		return(STR_UP + 4 * DM);
	}
	if (n == 10)
	{
		return(STR_UP + 5 * DM);
	}
	if (n == 11)
	{
		return(STR_UP + 6 * DM);    // STR
	}
	if (n == 12)
	{
		return(LIN_DOWN - 3 * DD);
	}
	if (n == 13)
	{
		return(LIN_DOWN - 2 * DD);
	}
	if (n == 14)
	{
		return(LIN_DOWN - 1 * DD);
	}
	if (n == 15)
	{
		return(LIN_DOWN - 0 * DD);    // LIN
	}
	return(0);
}
