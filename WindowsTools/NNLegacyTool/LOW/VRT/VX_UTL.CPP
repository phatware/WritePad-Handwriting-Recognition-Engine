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

#include "math.h"

#include "bastypes.h"
#include "vx_def.h"
#include "vx_utl.h"

_SHORT CopyTrace(p_SHORT x, p_SHORT y, _SHORT n, p_SHORT xReal, p_SHORT yReal, _SHORT nReal, _LONG nLength)
{
	_SHORT i, j, k, N = 0;
	_SHORT x1, y1, x2, y2, nPnt;

	for (i = 0; i < nReal; i++)
	{
		if (yReal[i] != VX_BREAK)
		{
			if (N >= n && n > 0)
			{
				return(-1);
			}

			if (n > 0)
			{
				x[N] = xReal[i];
			}
			if (n > 0)
			{
				y[N] = yReal[i];
			}
			N++;
		}
		else
		{
			if (i == 0)
			{
				k = 2;
				while (CalcLngSq(xReal[i + 1], yReal[i + 1], xReal[i + k], yReal[i + k]) <= nLength)
				{
					k++;
					if (i + k >= nReal)
					{
						return(-1);
					}
				}
				x1 = xReal[i + k];
				y1 = yReal[i + k];
				x2 = xReal[i + 1];
				y2 = yReal[i + 1];

				x1 = x1 + (x1 - x2) * 3;
				y1 = y1 + (y1 - y2) * 3;
			}
			else
				if (i == nReal - 1)
				{
					k = 2;
					while (CalcLngSq(xReal[i - 1], yReal[i - 1], xReal[i - k], yReal[i - k]) <= nLength)
					{
						k++;
						if (i - k <= 0)
						{
							return(-1);
						}
					}
					x2 = xReal[i - k];
					y2 = yReal[i - k];
					x1 = xReal[i - 1];
					y1 = yReal[i - 1];

					x2 = x2 + (x2 - x1) * 3;
					y2 = y2 + (y2 - y1) * 3;
				}
				else
				{
					x1 = xReal[i - 1];
					y1 = yReal[i - 1];
					x2 = xReal[i + 1];
					y2 = yReal[i + 1];
				}
			nPnt = (_SHORT) sqrt((double) CalcLngSq(x1, y1, x2, y2) / VX_HORDA) + 1;

			if (N >= n && n > 0)
			{
				return(-1);
			}

			if (n > 0)
			{
				y[N] = VX_BREAK;
			}
			if (n > 0)
			{
				x[N] = 0;
			}
			N++;
			for (j = 1; j < nPnt; j++)
			{
				if (N >= n && n > 0)
				{
					return(-1);
				}

				if (n > 0)
				{
					x[N] = (_SHORT) (x1 + ((_LONG) (x2 - x1)*j) / nPnt);
				}
				if (n > 0)
				{
					y[N] = (_SHORT) (y1 + ((_LONG) (y2 - y1)*j) / nPnt);
				}
				N++;
			}
			if (N >= n && n > 0)
			{
				return(-1);
			}

			if (n > 0)
			{
				y[N] = VX_BREAK;
			}
			if (n > 0)
			{
				x[N] = 1;
			}
			N++;
		}
	}
	return(N);
}

// Calculate length between two points (square)

_LONG CalcLngSq(_SHORT x1, _SHORT y1, _SHORT x0, _SHORT y0)
{
	_LONG dx1 = x1 - x0;
	_LONG dy1 = y1 - y0;

	return(dx1*dx1 + dy1*dy1);
}

// Calculate angle

_SHORT CalcAngle(_SHORT x0, _SHORT y0, _SHORT x1, _SHORT y1)
{
	_DOUBLE  dx = x1 - x0;
	_DOUBLE  dy = y1 - y0;
	_DOUBLE  sq = sqrt(dx*dx + dy*dy);
	_DOUBLE  DirA;

	if (sq == 0)
	{
		return(-1);
	}

	if (dy < 0)
	{
		dy = -dy;
	}

	DirA = asin(dy / sq)*VX_RAD;

	if (x1 < x0 && y1 >= y0)
	{
		DirA = 180 - DirA;
	}
	if (x1 < x0 && y1 < y0)
	{
		DirA = 180 + DirA;
	}
	if (x1 >= x0 && y1 < y0)
	{
		DirA = 360 - DirA;
	}

	return((_SHORT) DirA);
}

// Calculate angle between two lines

_SHORT CalcAng12(_SHORT x0, _SHORT y0, _SHORT x1, _SHORT y1, _SHORT x2, _SHORT y2)
{
	_SHORT AngA = CalcAngle(x0, y0, x1, y1) - CalcAngle(x0, y0, x2, y2);

	if (AngA < 0)
	{
		AngA = -AngA;
	}
	if (AngA > 180)
	{
		AngA = 360 - AngA;
	}

	return(AngA);
}

// Calculate clockwise or no

_BOOL CalcDirec(_SHORT x0, _SHORT y0, _SHORT x1, _SHORT y1, _SHORT x2, _SHORT y2)
{
	_SHORT AngA = CalcAngle(x0, y0, x1, y1) - CalcAngle(x0, y0, x2, y2);

	if (AngA > 180)
	{
		AngA -= 360;
	}
	if (AngA < -180)
	{
		AngA += 360;
	}

	return(AngA < 0);
}

// Calculate angle direction

_SHORT CalcDir12(_SHORT x0, _SHORT y0, _SHORT x1, _SHORT y1, _SHORT x2, _SHORT y2)
{
	_SHORT Ang1 = CalcAngle(x0, y0, x1, y1);
	_SHORT Ang2 = CalcAngle(x0, y0, x2, y2);
	_SHORT AngA = Ang1 - Ang2;
	_SHORT DirA;

	if (AngA < 0)
	{
		AngA = -AngA;
	}
	DirA = (Ang1 < Ang2 ? Ang1 : Ang2) + AngA / 2;

	if (AngA>180)
	{
		AngA = 360 - AngA;
		DirA += 180;
		if (DirA >= 360)
		{
			DirA -= 360;
		}
	}
	DirA += 180;
	if (DirA >= 360)
	{
		DirA -= 360;
	}
	if (DirA < 0)
	{
		DirA += 360;
	}

	return(DirA);
}

_DOUBLE HCos(_SHORT nAng)
{
	_DOUBLE dAng = nAng / VX_RAD;

	return(cos(dAng));
}

_DOUBLE HSin(_SHORT nAng)
{
	_DOUBLE dAng = nAng / VX_RAD;

	return(sin(dAng));
}
