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

#include "ams_mg.h"
#include "hwr_sys.h"

#include "orto.h"
#include "ortow.h"
#include "param.h"
#include "dscr.h"
#include "polyco.h"
#include "xr_names.h"


// ---------------------- Defines ----------------------------------------------

#define BREAK (-1)
#define FBO_ABS(x) (((x) >= 0) ? (x) : (-(x)))

// --------------------- Types -------------------------------------------------

typedef struct
{
	_INT   pnum;
	_INT   best_len;
	_UCHAR order[MAX_PARTS_IN_LETTER];
	_UCHAR best_order[MAX_PARTS_IN_LETTER];
	_INT   sx[MAX_PARTS_IN_LETTER];
	_INT   sy[MAX_PARTS_IN_LETTER];
	_INT   ex[MAX_PARTS_IN_LETTER];
	_INT   ey[MAX_PARTS_IN_LETTER];
	Part_of_letter pp[MAX_PARTS_IN_LETTER];
} fbo_type, _PTR p_fbo_type;

// -------------------- Proto --------------------------------------------------

p_3DPOINT XrdToTrace(xrdata_type _PTR xrdata, _TRACE fullTrace, _INT iBegXr, _INT iEndXr, p_INT pnPointsOut, p_INT num_strokes, p_RECT rect, Part_of_letter _PTR parts);
_INT      SortParts(_INT np, pPart_of_letter pparts, _TRACE trace);
_INT      FindBestOrder(_INT depth, _INT len, p_fbo_type fbo);

/* *************************************************************************** */
/* *     Get PolyCo given trace and XrData Range                             * */
/* *************************************************************************** */
_INT GetPolyCo(_INT st, _INT len, p_xrdata_type xrdata, _TRACE trace, p_UCHAR coeff)
{
	_INT     i, j, n;
	_INT     num_points, num_strokes;
	p_3DPOINT p_sym_trace;
	_3DPOINT Coeffs[PC_N_INT_COEFFS];
	_LONG    lLambda, lError = 0;
	_RECT    rect;
	Part_of_letter parts[MAX_PARTS_IN_LETTER + 1];


	p_sym_trace = XrdToTrace(xrdata, trace, st, st + len - 1, &num_points, &num_strokes, &rect, &parts[0]);
	if (p_sym_trace == _NULL)
	{
		goto err;
	}
	if (num_points < 3)
	{
		goto err;
	}

	if (!Trace3DToDct((_WORD) num_points, p_sym_trace, (_WORD) PC_N_INT_COEFFS, Coeffs,
	                  (_WORD) 1, (_WORD) 0, &lLambda, 0 /*&lError*/, _FALSE))
	{
		goto err;
	}
	for (i = 1, j = 0; j < PC_N_INT_COEFFS; i += 3, j++)
	{
		n = Coeffs[j].x + 128;
		if (n < 0)
		{
			n = 0;
		}
		if (n > 255)
		{
			n = 255;
		}
		coeff[i + 2] = (_UCHAR) n;
		n = Coeffs[j].y + 128;
		if (n < 0)
		{
			n = 0;
		}
		if (n > 255)
		{
			n = 255;
		}
		coeff[i + 1] = (_UCHAR) n;
		n = Coeffs[j].z + 128;
		if (n < 0)
		{
			n = 0;
		}
		if (n > 255)
		{
			n = 255;
		}
		coeff[i + 0] = (_UCHAR) n;
	}

	n = (num_strokes - 1) * 64;
	if (n > 255)
	{
		n = 255;
	}
	coeff[0] = (_UCHAR) n;      // Number of strokes
	n = 0;                                                                  // 'Separate' flag on the left and right, replaces 0 order Z
	if (IS_XR_LINK((*xrdata->xrd)[st - 1].xr.type))
	{
		n += (_UCHAR) 1;
	}
	if (IS_XR_LINK((*xrdata->xrd)[st + len - 1].xr.type))
	{
		n += (_UCHAR) 2;
	}
	coeff[1] = (_UCHAR) (n * 64);
	n = lError / 256;
	if (n < 0)
	{
		n = 0;
	}
	if (n > 255)
	{
		n = 255;
	}
	coeff[PC_NUM_COEFF - 1] = (_UCHAR) n; // Approximation error

	if (p_sym_trace)
	{
		HWRMemoryFree(p_sym_trace);
	}

	if (GetSnnBitMap(st, len, xrdata, trace, &coeff[PC_NUM_COEFF], &rect, &parts[0]))
	{
		goto err;
	}

	return 0;
err:
	if (p_sym_trace)
	{
		HWRMemoryFree(p_sym_trace);
	}
	return 1;
}

/* *************************************************************************** */
/* *     Get Trace for given xrdata range                                    * */
/* *************************************************************************** */
// ATTENTION!!!   If the result is successful (return value != NULL),
//              the caller function must free trace memory after usage!!!

p_3DPOINT XrdToTrace(xrdata_type _PTR xrdata, _TRACE fullTrace, _INT iBegXr, _INT iEndXr, p_INT pnPointsOut, p_INT num_strokes, p_RECT rect, Part_of_letter _PTR Parts)
{
	_INT             i, j;
	_INT             modified = 0;
	_SHORT           nParts = 0;
	_SHORT           x, y;
	//  Part_of_letter   Parts[MAX_PARTS_IN_LETTER];
	_INT             nPoints = 0;
	p_3DPOINT        pTrace = _NULL;
	p_xrd_el_type    xrd = (p_xrd_el_type) xrdata->xrd;


	//Check validity of input:
	if (iBegXr == 0 || iEndXr == 0 || fullTrace == NULL)
	{
		goto  EXIT_ACTIONS;
	}
	if (iBegXr < 0 || iEndXr >= XRINP_SIZE || iEndXr < iBegXr)
	{
		goto  EXIT_ACTIONS;
	}

	if (iBegXr > 2) // recover connecting trace (mainly for cursive 's')
	{
		if (!IsXrLink(&(*xrdata->xrd)[iBegXr]) && !GetXrMovable(&(*xrdata->xrd)[iBegXr]) &&
		        !IsXrLink(&(*xrdata->xrd)[iBegXr - 1]) && !GetXrMovable(&(*xrdata->xrd)[iBegXr - 1]))
		{
			iBegXr--;
			modified++;
		}
	}

	if (connect_trajectory_and_letter(xrd, (_SHORT) iBegXr, (_SHORT) iEndXr, (p_SHORT) &nParts, &Parts[0]) != SUCCESS)
	{
		goto  EXIT_ACTIONS;
	}
	if (nParts == 0)
	{
		goto EXIT_ACTIONS;
	}

	Parts[nParts].iend = 0;

	if (modified)
	{
		j = ((*xrdata->xrd)[iBegXr].endpoint + (*xrdata->xrd)[iBegXr + 1].begpoint) / 2;
		if (Parts[0].ibeg < j)
		{
			Parts[0].ibeg = (_SHORT) (j);
		}
		if (Parts[0].ibeg > Parts[0].iend)
		{
			Parts[0].ibeg = Parts[0].iend;
		}
	}

	SortParts(nParts, &Parts[0], fullTrace);

	//Calc. the number of points in all parts and alloc "pTrace" of that size:
	nPoints = 1;  //for possibly needed BREAK at beg.
	for (i = 0; i < nParts; i++)
	{
		nPoints += Parts[i].iend - Parts[i].ibeg + 1 + 1;  //"1" - for BREAK after parts
	}

	if (nPoints <= 0)
	{
		goto EXIT_ACTIONS;
	}

	pTrace = (p_3DPOINT) HWRMemoryAlloc((nPoints + nParts + 16)*sizeof(_3DPOINT));
	if (pTrace == _NULL)
	{
		goto  EXIT_ACTIONS;
	}

	rect->top = rect->left = 32000;
	rect->bottom = rect->right = 0;

	//Copy letter trace:

	for (i = nPoints = 0; i < nParts; i++)
	{
		//    for (j = 0; i > 0 && j < 3; j ++)
		if (i > 0)
		{
			pTrace[nPoints].x = 0;
			pTrace[nPoints].y = BREAK;
			pTrace[nPoints].z = 0;
			nPoints++;
		}

		for (j = Parts[i].ibeg; j <= Parts[i].iend; j++)
		{
			if (nPoints > 0 && fullTrace[j].y == BREAK && pTrace[nPoints - 1].y == BREAK)
			{
				continue;
			}

			pTrace[nPoints].x = x = fullTrace[j].x;
			pTrace[nPoints].y = y = fullTrace[j].y;
			pTrace[nPoints].z = 100;
			nPoints++;

			if (y >= 0)  // Escape problems with multistroke trajectory parts
			{
				if (x > rect->right)
				{
					rect->right = x;
				}
				if (x < rect->left)
				{
					rect->left = x;
				}
				if (y > rect->bottom)
				{
					rect->bottom = y;
				}
				if (y < rect->top)
				{
					rect->top = y;
				}
			}
		}
	}

	if (pTrace[nPoints - 1].y != BREAK)
	{
		pTrace[nPoints].x = 0;
		pTrace[nPoints].y = BREAK;
		pTrace[nPoints].z = 0;
		nPoints++;
	}

	for (i = 1; i < nPoints - 1; i++) // Glue strokes with 'air' trajectory
	{
		if (pTrace[i].y == BREAK)
		{
			pTrace[i + 0].x = (_SHORT) ((pTrace[i - 1].x + pTrace[i + 1].x) / 2);
			pTrace[i + 0].y = (_SHORT) ((pTrace[i - 1].y + pTrace[i + 1].y) / 2);
			pTrace[i + 0].z = (_SHORT) 200;

			pTrace[i - 1].z = 120;
			pTrace[i + 1].z = 120;
		}
	}


EXIT_ACTIONS:
	;

	*pnPointsOut = nPoints;
	*num_strokes = nParts;
	return  pTrace;
}

/* *************************************************************************** */
/* *     Sort parts to in order of writing                                   * */
/* *************************************************************************** */
_INT SortParts(_INT np, pPart_of_letter pparts, _TRACE trace)
{
	_INT i;
	_INT all_sorted;
	Part_of_letter pp;

	if (np < 2)
	{
		goto done;
	}

	all_sorted = 0;
	while (!all_sorted)
	{
		for (i = 1, all_sorted = 1; i < np; i++)
		{
			if (pparts[i - 1].ibeg > pparts[i].ibeg)
			{
				pp = pparts[i - 1];
				pparts[i - 1] = pparts[i];
				pparts[i] = pp;

				all_sorted = 0;
			}
		}
	}

done:
	return 0;
}
