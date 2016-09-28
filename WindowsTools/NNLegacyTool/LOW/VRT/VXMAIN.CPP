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
#include "vx_img.h"
#include "vx_fnc.h"
#include "vx_utl.h"
#include "vx_dbg.h"

#define IsBrk(i)         (y[i]==VX_BREAK)
#define CalcLngSq(x1,y1,x2,y2) (((x1)-(x2))*((x1)-(x2))+((y1)-(y2))*((y1)-(y2)))

// ********** DEBUG Section ***********************************************************************
#if PG_DEBUG
#include "pg_debug.h"
#include "wlink.h"
#define   Stop(Text) {brkeyw(Text);}
#endif
// ************************************************************************************************
static _BOOL  InitBeg();                        // memory allocation & initialization
static _VOID  InitEnd();                        // memory release
static _VOID  Calculate(_SHORT nPass);          // calculate angles
static _SHORT CalcPoint(_SHORT i0);             // first  pass angles calculation
static _SHORT CalcExtrm(_SHORT i0);             // next passes angles calculation
static _VOID  SmoothAng(p_SHORT, p_SHORT);       // smooth angles
static void   FindAngEx(p_SHORT, _SHORT aMax);   // find extremes
static void   FindZones(p_SHORT);               // find zones between extremes
static void   FindPlace(p_SHORT, _SHORT);        // move extremes to bestest places
static void   FindElems();                      // find elements
// ********** Static data section *****************************************************************
static  p_low_type  pData;  // low level data
static  p_SHORT     Ready;  // extremes, TEARS, ZONES information (look ZONE,TEAR,EXTR & NONE)
static  p_SHORT     AnglT;  // average angle
static  p_SHORT     Angl1;  // short  base angle
static  p_SHORT     Angl2;  // medium base angle
static  p_SHORT     Angl3;  // long base angle
static  p_SHORT     Direc;  // directions
static  p_SHORT     X1;     // left  edge of extreme point
static  p_SHORT     X2;     // right edge of extreme point
static  p_SHORT     xTraj;  // scaled,filtered trajectory (x)
static  p_SHORT     yTraj;  // scaled,filtered trajectory (y)
static   _SHORT     nTraj;  // number of points
static  p_SHORT     x;      // modified x-coordinates
static  p_SHORT     y;      // modified y-coordinates
static   _SHORT     N;      // number of points

static p_SHORT PntGetX();
static p_SHORT PntGetY();
static  _SHORT PntGetN();
static  _VOID  PntFree();
static  _SHORT PntSkip(p_SHORT x, p_SHORT y, _SHORT n, _SHORT i);
static  _BOOL  PntCopy(p_SHORT xTraj, p_SHORT yTraj, _SHORT nTraj);
// ************************************************************************************************
// ********** MAIN Monitor ************************************************************************
_BOOL VxMonitor(p_low_type low_data, p_xrdata_type xrdata)
{
	int     i;

	if (xrdata->xrd == _NULL)
	{
		MessageBox(_NULL, "Empty XRD in VxMonitor entry...", _NULL, MB_OK || MB_ICONSTOP);
		return(_FALSE);
	}
	pData = low_data;

	//DTime(TIME_BEG,_NULL);
	if (!InitBeg())
	{
		return(_FALSE);
	}

	SetXrElemPtr(xrdata);

#if PG_DEBUG
	if (mpr >= 30 && mpr < 40)
	{
		draw_arc(EGA_WHITE, x, y, (_SHORT) 0, (_SHORT) (N - 1));
	}
#endif
	/***** Calculations *****/
	Calculate(PASS1);

	SmoothAng(Angl1, AnglT);
	SmoothAng(Angl2, AnglT);
	SmoothAng(Angl3, AnglT);

	for (i = 0; i < N; i++)
	{
		AnglT[i] = (Angl1[i] + Angl2[i] + Angl3[i]) / 3;
		Ready[i] = NONE;
	}
	FindAngEx(Angl1, 28);
	FindAngEx(Angl2, 28);
	FindAngEx(Angl3, 28);
	FindPlace(AnglT, 21);

	Calculate(PASS2);

	FindZones(AnglT);
	Calculate(PASS3);
	for (i = 0; i < N; i++) if (Ready[i] == ZONE)
		{
			Ready[i] = NONE;
		}
	FindZones(AnglT);
#if PG_DEBUG
	if (mpr != 34)
	{
		Calculate(PASS4);
	}
#endif
	FindElems();

#if  PG_DEBUG
	DrawAll(pData, x, y, N);
	ShowAngls(Ready, Direc, Angl1, Angl1, Angl3, AnglT, N);
	if (mpr >= 30 && mpr < 40)
	{
		ShowReady(Ready, X1, X2, x, y, N);
	}
#endif
	InitEnd();
	return(_TRUE);
}
// ************************************************************************************************
// ************************************************************************************************

_BOOL InitBeg()
{
	_BOOL  bRet = _TRUE;
	_ULONG S;
#if PG_DEBUG
	wLinkRegister("None");
	SD_Delete(-1);
#endif

	if (!PntCopy(pData->x, pData->y, pData->ii))
	{
		return _FALSE;
	}

	nTraj = PntGetN();
	xTraj = PntGetX();
	yTraj = PntGetY();

	N = CopyTrace(x, y, 0, xTraj, yTraj, nTraj, VMAX);
	S = N*sizeof(_SHORT);
	if ((x = (p_SHORT) HWRMemoryAlloc(S)) == _NULL)
	{
		bRet = _FALSE;
	}
	if ((y = (p_SHORT) HWRMemoryAlloc(S)) == _NULL)
	{
		bRet = _FALSE;
	}
	N = CopyTrace(x, y, N, xTraj, yTraj, nTraj, VMAX);
	if (N < 0)
	{
		bRet = _FALSE;
		N = 1;
	}
	PntFree();

	S = N*sizeof(_SHORT);
	if ((Ready = (p_SHORT) HWRMemoryAlloc(S)) == _NULL)
	{
		bRet = _FALSE;
	}
	if ((AnglT = (p_SHORT) HWRMemoryAlloc(S)) == _NULL)
	{
		bRet = _FALSE;
	}
	if ((Angl1 = (p_SHORT) HWRMemoryAlloc(S)) == _NULL)
	{
		bRet = _FALSE;
	}
	if ((Angl2 = (p_SHORT) HWRMemoryAlloc(S)) == _NULL)
	{
		bRet = _FALSE;
	}
	if ((Angl3 = (p_SHORT) HWRMemoryAlloc(S)) == _NULL)
	{
		bRet = _FALSE;
	}
	if ((Direc = (p_SHORT) HWRMemoryAlloc(S)) == _NULL)
	{
		bRet = _FALSE;
	}
	if ((X1 = (p_SHORT) HWRMemoryAlloc(S)) == _NULL)
	{
		bRet = _FALSE;
	}
	if ((X2 = (p_SHORT) HWRMemoryAlloc(S)) == _NULL)
	{
		bRet = _FALSE;
	}

	if (!bRet)
	{
		InitEnd();
	}
	return(bRet);
}

void InitEnd()
{
	if (Ready != _NULL)
	{
		HWRMemoryFree(Ready);
	}
	if (AnglT != _NULL)
	{
		HWRMemoryFree(AnglT);
	}
	if (Angl1 != _NULL)
	{
		HWRMemoryFree(Angl1);
	}
	if (Angl2 != _NULL)
	{
		HWRMemoryFree(Angl2);
	}
	if (Angl3 != _NULL)
	{
		HWRMemoryFree(Angl3);
	}
	if (Direc != _NULL)
	{
		HWRMemoryFree(Direc);
	}
	if (X1 != _NULL)
	{
		HWRMemoryFree(X1);
	}
	if (X2 != _NULL)
	{
		HWRMemoryFree(X2);
	}
	if (x != _NULL)
	{
		HWRMemoryFree(x);
	}
	if (y != _NULL)
	{
		HWRMemoryFree(y);
	}
}

// ********** Calculate angles **********************************

_VOID Calculate(_SHORT nPass)
{
	_SHORT i0;

	if (nPass == PASS1)
	{
		for (i0 = 0; i0 < N; i0++) if (IsBrk(i0) || CalcPoint(i0) < 0)
			{
				Angl1[i0] = Angl2[i0] = Angl3[i0] = NONE;
			}
	}
	else for (i0 = 0; i0 < N; i0++) if (Ready[i0] == EXTR)
			{
				CalcExtrm(i0);
			}
}

_SHORT CalcPoint(_SHORT i0)
{
	_SHORT  AA;
	_SHORT  i1 = i0;
	_SHORT  i2 = i0;
	_SHORT  lMax = VMAX;
	_SHORT  l1 = 0;
	_SHORT  l2 = 0;
	_SHORT  aa_old, dx, l1_old, l2_old, dx_old;

	l1_old = 0;
	l2_old = 0;
	while (l1 <= lMax)
	{
		do
		{
			if (--i1 <= 0 || l1 < l1_old)
			{
				return(-1);
			}
		}
		while (IsBrk(i1));
		l1_old = l1;
		l1 = CalcLngSq(x[i0], y[i0], x[i1], y[i1]);
	}
	while (l2 <= lMax)
	{
		do
		{
			if (++i2 >= N - 1 || l2 < l2_old)
			{
				return(-1);
			}
		}
		while (IsBrk(i2));
		l2_old = l2;
		l2 = CalcLngSq(x[i0], y[i0], x[i2], y[i2]);
	}
	AA = CalcAng12(x[i0], y[i0], x[i1], y[i1], x[i2], y[i2]);

	Angl1[i0] = AA;

	aa_old = AA + 15;
	if (aa_old < 25)
	{
		aa_old = 25;
	}
	dx_old = dx = CalcLngSq(x[i1], y[i1], x[i2], y[i2]) / 4;

	for (lMax = MMAX;; lMax = XMAX)
	{
		while (l1 <= lMax || l2 <= lMax)
		{
			if (l1 < l2) do
				{
					i1--;
					if (i1 <= 0 || dx < dx_old || l1<l1_old || AA>aa_old)
					{
						if (IsBrk(i1))
						{
							i1++;
						}
						goto Done;
					}
				}
				while (IsBrk(i1));
			else      do
				{
					i2++;
					if (i2 >= N - 1 || dx < dx_old || l2<l2_old || AA>aa_old)
					{
						if (IsBrk(i2))
						{
							i2--;
						}
						goto Done;
					}
				}
				while (IsBrk(i2));
			l1_old = l1;
			l2_old = l2;

			l1 = CalcLngSq(x[i0], y[i0], x[i1], y[i1]);
			l2 = CalcLngSq(x[i0], y[i0], x[i2], y[i2]);
			dx = CalcLngSq(x[i1], y[i1], x[i2], y[i2]);
			AA = CalcAng12(x[i0], y[i0], x[i1], y[i1], x[i2], y[i2]);
		}
Done:
		if (lMax == MMAX)
		{
			Angl2[i0] = l1 >= MMAX / 2 ? CalcAng12(x[i0], y[i0], x[i1], y[i1], x[i2], y[i2]) : NONE;
		}
		else
		{
			Angl3[i0] = l1 >= MMAX ? CalcAng12(x[i0], y[i0], x[i1], y[i1], x[i2], y[i2]) : NONE;
			break;
		}
	}
	return(0);
}

_SHORT CalcExtrm(_SHORT i0)
{
	_SHORT  AA;
	_SHORT  i1 = i0;
	_SHORT  i2 = i0;
	_SHORT  lMax = XMAX;
	_SHORT  l1 = 0;
	_SHORT  l2 = 0;
	_SHORT  aa_old, dx, l1_old, l2_old, dx_old;
	_BOOL   bFlg = _TRUE;

	l1_old = 0;
	l2_old = 0;
	aa_old = AA = NONE;
	dx_old = dx = 0;

	while (l1 <= lMax || l2 <= lMax)
	{
		if (l1 < l2) do
			{
				i1--;
				if (i1 <= 0 || dx < dx_old || l1<l1_old || AA>aa_old || Ready[i1] == ZONE || Ready[i1] == EXTR)
				{
					if (IsBrk(i1))
					{
						i1++;
					}
					goto Done;
				}
			}
			while (IsBrk(i1));
		else      do
			{
				i2++;
				if (i2 >= N - 1 || dx < dx_old || l2<l2_old || AA>aa_old || Ready[i2] == ZONE || Ready[i2] == EXTR)
				{
					if (IsBrk(i2))
					{
						i2--;
					}
					goto Done;
				}
			}
			while (IsBrk(i2));
		l1_old = l1;
		l2_old = l2;
		if (bFlg && l1 > VMAX)
		{
			dx_old = dx / 4;
			aa_old = AA + 15;
			if (aa_old < 25)
			{
				aa_old = 25;
			}
			bFlg = _FALSE;
		}
		l1 = CalcLngSq(x[i0], y[i0], x[i1], y[i1]);
		l2 = CalcLngSq(x[i0], y[i0], x[i2], y[i2]);
		dx = CalcLngSq(x[i1], y[i1], x[i2], y[i2]);
		AA = CalcAng12(x[i0], y[i0], x[i1], y[i1], x[i2], y[i2]);
	}
Done:
	AnglT[i0] = CalcAng12(x[i0], y[i0], x[i1], y[i1], x[i2], y[i2]);
	X1[i0] = i1;
	X2[i0] = i2;
	return(0);
}

_VOID  SmoothAng(p_SHORT pAngle, p_SHORT pBuffer)
{
	_SHORT i, AL5, AL4, AL3, AL2, AL1, AA0, AR1, AR2, AR3, AR4, AR5;

	for (i = 5; i < N - 5; i++)
	{
		AA0 = pAngle[i];

		AL1 = pAngle[i - 1];
		AL2 = pAngle[i - 2];
		AL3 = pAngle[i - 3];
		AL4 = pAngle[i - 4];
		AL5 = pAngle[i - 5];

		AR1 = pAngle[i + 1];
		AR2 = pAngle[i + 2];
		AR3 = pAngle[i + 3];
		AR4 = pAngle[i + 4];
		AR5 = pAngle[i + 5];

		if (IsBrk(i))
		{
			AA0 = (AL1 + AR1) / 2;
		}
		else
		{
			AA0 = ((AA0 + (AL1 + AR1) / 2 + (AL2 + AR2) / 4 + (AL3 + AR3) / 8 + (AL4 + AR4) / 16 + (AL5 + AR5) / 32));
			AA0 = (_SHORT) (AA0 / (1.0 + 2.0 / 2 + 2.0 / 4 + 2.0 / 8 + 2.0 / 16 + 2.0 / 32));
		}

		pBuffer[i] = AA0;
	}
	for (i = 5; i < N - 5; i++)
	{
		pAngle[i] = pBuffer[i];
	}
	for (i = 0; i < 5; i++)
	{
		pAngle[i] = NONE;
	}
	for (i = N - 5; i < N; i++)
	{
		pAngle[i] = NONE;
	}
}

void FindAngEx(p_SHORT pAngle, _SHORT aMax)
{
	_SHORT k, i, j;
	_SHORT aMaxL, aMinX, aMaxR;
	_SHORT iMaxL, iMinX, iMaxR, iOldX = 0;

	aMaxL = 180;
	iMaxL = 0;
	aMinX = 180;
	iMinX = 0;

	for (i = 0; i < N;)
	{
		do
		{
			k = pAngle[i];
			if (k < aMinX)
			{
				aMinX = k;
				iMinX = i;
			}
			if (k > aMaxL)
			{
				aMaxL = k;
				iMaxL = i;
				aMinX = k;
				iMinX = i;
			}
			if (++i >= N)
			{
				goto Next;
			}
		}
		while (aMaxL - aMinX < aMax);

		aMaxR = aMinX;
		iMaxR = iMinX;

		do
		{
			k = pAngle[i];
			if (k < aMinX)
			{
				aMinX = k;
				iMinX = i;
				aMaxR = k;
				iMaxR = i;
			}
			if (k > aMaxR)
			{
				aMaxR = k;
				iMaxR = i;
			}
			if (++i >= N)
			{
				break;
			}
		}
		while (aMaxR - aMinX < aMax);

		if (pAngle[iMinX] > 100)
		{
			aMaxL = aMaxR = aMinX;
			iMaxL = iMaxR = iMinX;

			for (j = iMinX + 1; j < N; j++)
			{
				k = pAngle[j];
				if (aMaxR - aMinX >= aMax)
				{
					break;
				}
				if (k > aMaxR)
				{
					aMaxR = k;
					iMaxR = j;
				}
			}
			for (j = iMinX - 1; j >= 0; j--)
			{
				k = pAngle[j];
				if (aMaxL - aMinX >= aMax)
				{
					break;
				}
				if (k > aMaxL)
				{
					aMaxL = k;
					iMaxL = j;
				}
			}
			iMinX = (iMaxL + iMaxR) / 2;
		}
		if (IsBrk(iMinX))
		{
			iMinX--;
			if (iMinX < 0)
			{
				iMinX += 2;
			}
		}
		Ready[iMinX] = EXTR;
		X1[iMinX] = iMaxL;
		X2[iMinX] = iMaxR;
		aMaxL = aMaxR;
		iMaxL = iMaxR;
		aMinX = aMaxR;
		iMinX = iMaxR;
	}
Next:
	;
}

void FindZones(p_SHORT pAngle)
{
	_SHORT iNew, iOld, nNew, nOld, nMid;

	for (iOld = -1, iNew = 0; iNew < N; iNew++)
	{
		_SHORT nCode = Ready[iNew];
		_LONG  l1, l2;

		if (Ready[iNew] != EXTR)
		{
			continue;
		}

		if (iOld >= 0)
		{
			nOld = X2[iOld];
			nNew = X1[iNew];

			nMid = nOld + (nNew - nOld) / 2;
			l1 = CalcLngSq(x[iOld], y[iOld], x[nMid], y[nMid]);
			l2 = CalcLngSq(x[iNew], y[iNew], x[nMid], y[nMid]);

			if (l1 <= VMAX && l2 <= VMAX)
			{
				nMid = (iOld + iNew) / 2;
			}
			else
				if (l1 <= VMAX)
				{
					while (l1 < l2)
					{
						while (IsBrk(++nMid)) /*DO*/;
						l1 = CalcLngSq(x[iOld], y[iOld], x[nMid], y[nMid]);
						l2 = CalcLngSq(x[iNew], y[iNew], x[nMid], y[nMid]);
					}
				}
				else
					if (l2 <= VMAX)
					{
						while (l1 > l2)
						{
							while (IsBrk(--nMid)) /*DO*/;
							l1 = CalcLngSq(x[iOld], y[iOld], x[nMid], y[nMid]);
							l2 = CalcLngSq(x[iNew], y[iNew], x[nMid], y[nMid]);
						}
					}
			Ready[nMid] = ZONE;
		}
		iOld = iNew;
	}
}

void FindPlace(p_SHORT pAngle, _SHORT aMax)
{
	_SHORT k, i, j;
	_SHORT aMaxL, aMinX, aMaxR;
	_SHORT iMaxL, iMinX, iMaxR, iOldX = 0;

	aMaxL = 180;
	iMaxL = 0;
	aMinX = 180;
	iMinX = 0;

	for (i = 0; i < N;)
	{
		do
		{
			if (IsBrk(i))
			{
				if (++i >= N)
				{
					goto Next;
				}
				else
				{
					continue;
				}
			}
			k = pAngle[i];
			if (k < aMinX)
			{
				aMinX = k;
				iMinX = i;
			}
			if (k > aMaxL)
			{
				aMaxL = k;
				iMaxL = i;
				aMinX = k;
				iMinX = i;
			}
			if (++i >= N)
			{
				goto Next;
			}
		}
		while (aMaxL - aMinX < aMax);

		aMaxR = aMinX;
		iMaxR = iMinX;

		do
		{
			if (IsBrk(i))
			{
				if (++i >= N)
				{
					goto Next;
				}
				else
				{
					continue;
				}
			}
			k = pAngle[i];
			if (k < aMinX)
			{
				aMinX = k;
				iMinX = i;
				aMaxR = k;
				iMaxR = i;
			}
			if (k > aMaxR)
			{
				aMaxR = k;
				iMaxR = i;
			}
			if (++i >= N)
			{
				break;
			}
		}
		while (aMaxR - aMinX < aMax);

		if (IsBrk(iMinX))
		{
			iMinX--;
			if (iMinX < 0)
			{
				iMinX += 2;
			}
		}
		_BOOL bFlg = _FALSE;
		int   j;

		for (j = iMaxL; j <= iMaxR; j++)
		{
			if (Ready[j] == EXTR)
			{
				bFlg = _TRUE;
				Ready[j] = NONE;
			}
		}
		if (bFlg)
		{
			Ready[iMinX] = ZONE;
			X1[iMinX] = iMaxL;
			X2[iMinX] = iMaxR;
		}
		aMaxL = aMaxR;
		iMaxL = iMaxR;
		aMinX = aMaxR;
		iMinX = iMaxR;
	}
Next:
	for (j = 0; j < N; j++)
	{
		if (Ready[j] == ZONE)
		{
			Ready[j] = EXTR;
		}
		else
		{
			Ready[j] = NONE;
		}
	}
}

void FindElems()
{
	p_SHORT xBuf = pData->xBuf;
	p_SHORT yBuf = pData->yBuf;
	_SHORT  nTraj = pData->rc->ii;
	_SHORT  i, j, k, j0, j1, j2;
	_BOOL   bFlg = _FALSE;
	p_SHORT iBack = pData->buffers[2].ptr;
	xrd_el_type _PTR xrelem;
	_RECT   rect;
	_SHORT  i1, i2;

	ElemAdd((_SHORT) NONE, x, y, (_SHORT) 0, (_SHORT) 0, (_SHORT) (N - 1), (_SHORT) 0, (_SHORT) 0, (_SHORT) 0, (_SHORT) 0); // Reset

	i1 = i2 = -1;

	for (i = 0, j = 0; i < N; i++)
	{
		if (IsBrk(i))
		{
			int k;

			bFlg = x[i];
			Ready[i] = TEAR;

			if (!bFlg)
			{
				i1 = i;
				i2 = N;
				for (k = i1 + 1; k < N; k++) if (IsBrk(k) && x[k])
					{
						i2 = k;
						break;
					}
			}
		}
		if (i == (i1 + i2) / 2 && i > 0 && i < N)
		{
			ElemAdd((_SHORT) TEAR, x, y, i1, (_SHORT) ((i1 + i2) / 2), i2, j, j, j, (_SHORT) 0);
		}

		if (Ready[i] == EXTR)
		{
			_SHORT i1 = X1[i];
			_SHORT i2 = X2[i];
			_SHORT nT;

			j0 = iBack[j];
			if (!bFlg) for (k = 1;; k++)
				{
					if (k > 0 && IsBrk(i - k))
					{
						j--;
						break;
					}
					if (k < N - 1 && IsBrk(i + k))
					{
						j++;
						break;
					}
				}
			nT = 0;
			for (k = i, j1 = j; k >= i1; k--, j1--) if (IsBrk(k))
				{
					j1++;
					nT += 1;
					break;
				}
			j1 = iBack[j1];
			for (k = i, j2 = j; k <= i2; k++, j2++) if (IsBrk(k))
				{
					j2--;
					nT += 2;
					break;
				}
			j2 = iBack[j2];

			if (CalcDirec(x[i], y[i], x[X1[i]], y[X1[i]], x[X2[i]], y[X2[i]]))
			{
				ElemAdd((_SHORT) -EXTR, x, y, i1, i, i2, j1, j0, j2, nT);
			}
			else
			{
				ElemAdd((_SHORT) EXTR, x, y, i1, i, i2, j1, j0, j2, nT);
			}
		}
		if (bFlg)
		{
			j++;
		}
	}

	for (xrelem = GetXrElemPtr(), i = 0; xrelem && i<GetXrElemNum(); i++)
	{
		GetTraceBox(xBuf, yBuf, xrelem[i].begpoint, xrelem[i].endpoint, (p_RECT) &rect);
		xrelem[i].box_left = rect.left;
		xrelem[i].box_up = rect.top;
		xrelem[i].box_right = rect.right;
		xrelem[i].box_down = rect.bottom;
	}
}

//*************************************************************************************************
//*************************************************************************************************

#define sCmp(a,b,c) ((a)>(((STR_DOWN-STR_UP)*b)/c))

static p_SHORT xPnt;
static p_SHORT yPnt;
static  _SHORT nPnt;

p_SHORT PntGetX()
{
	return xPnt;
}

p_SHORT PntGetY()
{
	return yPnt;
}

_SHORT PntGetN()
{
	return nPnt;
}

_VOID PntFree()
{
	if (xPnt)
	{
		HWRMemoryFree(xPnt);
	}
	if (yPnt)
	{
		HWRMemoryFree(yPnt);
	}

	xPnt = yPnt = _NULL;
	nPnt = 0;
}


_SHORT PntSkip(p_SHORT x, p_SHORT y, _SHORT n, _SHORT i)
{
	_SHORT ibeg = i;
	_SHORT iend = ibeg + 1;
	_RECT  rect = { ALEF, ALEF, 0, 0 };

	while (iend < n)
	{
		if (y[iend] == BREAK)
		{
			_SHORT dx, dy;

			if (rect.left == ALEF || rect.top == ALEF)
			{
				return(-1);
			}

			dx = rect.right - rect.left;
			dy = rect.bottom - rect.top;

			if (sCmp(dx, 1, 1))
			{
				return(-1);
			}
			if (sCmp(dy, 2, 3))
			{
				return(-1);
			}

			return(iend);
		}
		if (x[iend] < rect.left)
		{
			rect.left = x[iend];
		}
		if (x[iend] > rect.right)
		{
			rect.right = x[iend];
		}
		if (y[iend] < rect.top)
		{
			rect.top = y[iend];
		}
		if (y[iend] > rect.bottom)
		{
			rect.bottom = y[iend];
		}

		iend++;
	}
	return(-1);
}

_BOOL PntCopy(p_SHORT xTraj, p_SHORT yTraj, _SHORT nTraj)
{
	_ULONG S = nTraj * sizeof(_SHORT);
	_BOOL  bRet = _TRUE;
	_SHORT i, iJmp;

	if ((xPnt = (p_SHORT) HWRMemoryAlloc(S)) == _NULL)
	{
		bRet = _FALSE;
	}
	if ((yPnt = (p_SHORT) HWRMemoryAlloc(S)) == _NULL)
	{
		bRet = _FALSE;
	}
	if (!bRet)
	{
		PntFree();
		return bRet;
	}
	for (i = 0, nPnt = 0; i < nTraj; i++)
	{
		if (yTraj[i] < 0)
		{
			iJmp = PntSkip(xTraj, yTraj, nTraj, i);
			if (iJmp > 0)
			{
				i = iJmp - 1;
				continue;
			}
		}
		xPnt[nPnt] = xTraj[i];
		yPnt[nPnt] = yTraj[i];
		nPnt++;
	}
	return bRet;
}
