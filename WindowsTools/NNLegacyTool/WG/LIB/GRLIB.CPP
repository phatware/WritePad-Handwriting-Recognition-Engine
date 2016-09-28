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
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <grlib.h>
#include "WGDBG.H"
/* ------------------------------------------------------ */

void glbInit(HWND hwnd, HDC hDCPrinter,
             LP_GLB_DATA_TYPE glbBlock)
{
	/* if hwnd == 0 then it is a printer initialization */
	int     mmWidth, mmHeight, pxlsWidth, pxlsHeight;
	glbBlock->hwnd = hwnd;
	glbBlock->hDCPrinter = hDCPrinter;
	if (hwnd == 0 && hDCPrinter != NULL)   /* printer */
	{
		mmWidth = GetDeviceCaps(hDCPrinter, HORZSIZE);
		mmHeight = GetDeviceCaps(hDCPrinter, VERTSIZE);
		pxlsWidth = GetDeviceCaps(hDCPrinter, HORZRES);
		pxlsHeight = GetDeviceCaps(hDCPrinter, VERTRES);
		SetRect(&glbBlock->ClientRect, 0, 0, pxlsWidth, pxlsHeight);
		SetRect(&glbBlock->wRect, 0, 0, mmWidth, mmHeight);
	}
	else
		if (hwnd != 0)
		{
			GetClientRect(hwnd, &(glbBlock->ClientRect)); /* display */
			CopyRect(&glbBlock->wRect, &glbBlock->vRect);
		}
		else
		{
			SetRect(&glbBlock->ClientRect, 0, 0, INT_MAX, INT_MAX);
			SetRect(&glbBlock->wRect, 0, 0, INT_MAX, INT_MAX);
		}
	CopyRect(&glbBlock->vRect, &glbBlock->ClientRect);
	glbSetMapping(glbBlock);
} /* end of glbInit */

/* ------------------------------------------------------ */

void glbSetMapping(LP_GLB_DATA_TYPE glbBlock)
{
	int dxv, dyv, dxw, dyw;

	glbBlock->wOrg.x = (glbBlock->wRect.right + glbBlock->wRect.left) / 2;
	glbBlock->wOrg.y = (glbBlock->wRect.bottom + glbBlock->wRect.top) / 2;
	glbBlock->vOrg.x = (glbBlock->vRect.right + glbBlock->vRect.left) / 2;
	glbBlock->vOrg.y = (glbBlock->vRect.bottom + glbBlock->vRect.top) / 2;

	dxw = glbBlock->wRect.right - glbBlock->wRect.left;
	dyw = glbBlock->wRect.bottom - glbBlock->wRect.top;
	dxv = glbBlock->vRect.right - glbBlock->vRect.left;
	dyv = glbBlock->vRect.bottom - glbBlock->vRect.top;

	if (dxw == 0 || dyw == 0 || dxv == 0 || dyv == 0)   /* worng Viewport or Window */
	{
		glbBlock->wExt.x = 1;
		glbBlock->wExt.y = 1;
		glbBlock->vExt.x = 1;
		glbBlock->vExt.y = 1;
		return;
	}

	glbBlock->wExt.x = dxw;
	glbBlock->wExt.y = dyw;
	if (labs(dxw) * labs(dyv) > labs(dxv) * labs(dyw))   /* Window is "wider" */
	{
		glbBlock->vExt.x = dxv;
		glbBlock->vExt.y = MulDiv(dyw, dxv, dxw);
	}
	else
	{
		glbBlock->vExt.y = dyv;
		glbBlock->vExt.x = MulDiv(dxw, dyv, dyw);
	}
} /* end of glbSetMapping */

/* ------------------------------------------------------ */

void glbViewport(LP_GLB_DATA_TYPE glbBlock, LPRECT vRect)
{
	glbBlock->vRect.left = vRect->left;
	glbBlock->vRect.right = vRect->right;
	glbBlock->vRect.top = vRect->top;
	glbBlock->vRect.bottom = vRect->bottom;
	glbSetMapping(glbBlock);
} /* end of glbViewport */

/* ------------------------------------------------------ */

void glbWindow(LP_GLB_DATA_TYPE glbBlock, LPRECT wRect)
{
	glbBlock->wRect.left = wRect->left;
	glbBlock->wRect.right = wRect->right;
	glbBlock->wRect.top = wRect->top;
	glbBlock->wRect.bottom = wRect->bottom;
	glbSetMapping(glbBlock);
} /* end of glbWindow */

/* ------------------------------------------------------ */

void glbWindowToViewport(LP_GLB_DATA_TYPE glbBlock,
                         int xw, int yw, int FAR *xv, int FAR *yv)
{
	*xv = MulDiv(xw - glbBlock->wOrg.x, glbBlock->vExt.x, glbBlock->wExt.x) +
	      glbBlock->vOrg.x;
	*yv = MulDiv(yw - glbBlock->wOrg.y, glbBlock->vExt.y, glbBlock->wExt.y) +
	      glbBlock->vOrg.y;
} /* end of glbWindowToViewport */

/* ------------------------------------------------------ */

void glbViewportToWindow(LP_GLB_DATA_TYPE glbBlock,
                         int xv, int yv, int FAR *xw, int FAR *yw)
{
	*xw = MulDiv(xv - glbBlock->vOrg.x, glbBlock->wExt.x, glbBlock->vExt.x) +
	      glbBlock->wOrg.x;
	*yw = MulDiv(yv - glbBlock->vOrg.y, glbBlock->wExt.y, glbBlock->vExt.y) +
	      glbBlock->wOrg.y;
} /* end of glbViewporTtoWindow */

/* ------------------------------------------------------ */

void glbPolyline(LP_GLB_DATA_TYPE glbBlock,
                 HDC hDC, LPPOINT lpwPt, int numPoints)
{
	HGLOBAL h;
	LPPOINT lpvPt;
	int     i;
	h = DebugAlloc(GHND, numPoints * sizeof(POINT), "GlbPolyLine");
	if (h == NULL)
	{
		return;
	}
	lpvPt = (LPPOINT) DebugLockHandle(h);
	for (i = 0; i < numPoints; i++)
	{
		lpvPt[i].x = MulDiv(lpwPt[i].x - glbBlock->wOrg.x,
		                    glbBlock->vExt.x, glbBlock->wExt.x) +
		             glbBlock->vOrg.x;
		lpvPt[i].y = MulDiv(lpwPt[i].y - glbBlock->wOrg.y,
		                    glbBlock->vExt.y, glbBlock->wExt.y) +
		             glbBlock->vOrg.y;
	}
	if (numPoints > 1)
	{
		Polyline(hDC, lpvPt, numPoints);
	}
	else
		if (numPoints == 1)
		{
			HPEN       hPen;
			LOGPEN     lPen;
			// draw pixel
			hPen = (HPEN) SelectObject(hDC, GetStockObject(WHITE_PEN));
			GetObject(hPen, sizeof(LOGPEN), &lPen);
			SelectObject(hDC, hPen);
			SetPixel(hDC, lpvPt[0].x, lpvPt[0].y, lPen.lopnColor);
		}
	DebugUnlockHandle(h);
	DebugFree(h, "");
} /* end of glbPolyline */

/* ------------------------------------------------------ */

void glbMoveTo(LP_GLB_DATA_TYPE glbBlock, HDC hDC,
               int xw, int yw)
{

	MoveToEx(hDC, MulDiv(xw - glbBlock->wOrg.x,
	                     glbBlock->vExt.x, glbBlock->wExt.x) +
	         glbBlock->vOrg.x,
	         MulDiv(yw - glbBlock->wOrg.y,
	                glbBlock->vExt.y, glbBlock->wExt.y) +
	         glbBlock->vOrg.y, NULL);
} /* end of glbMoveTo */

/* ------------------------------------------------------ */

void glbLineTo(LP_GLB_DATA_TYPE glbBlock, HDC hDC,
               int xw, int yw)
{

	LineTo(hDC, MulDiv(xw - glbBlock->wOrg.x,
	                   glbBlock->vExt.x, glbBlock->wExt.x) +
	       glbBlock->vOrg.x,
	       MulDiv(yw - glbBlock->wOrg.y,
	              glbBlock->vExt.y, glbBlock->wExt.y) +
	       glbBlock->vOrg.y);
} /* end of glbLineTo */

/* ------------------------------------------------------ */


void glbPlot(LP_GLB_DATA_TYPE glbBlock, HDC hDC,
             LP_DATA_POINT_TYPE lpData, int NumPoints,
             LPPOINT lpAxisOrg, int dxMark, int dyMark)
{
	int   i, xt, yt;
	RECT  wBBox, vBBox;
	wBBox.left = INT_MAX;
	wBBox.right = INT_MIN;
	wBBox.top = INT_MAX;
	wBBox.bottom = INT_MIN;
	for (i = 0; i < NumPoints; i++)
	{
		if (lpData[i].x > wBBox.right)
		{
			wBBox.right = lpData[i].x;
		}
		if (lpData[i].x < wBBox.left)
		{
			wBBox.left = lpData[i].x;
		}
		if (lpData[i].y > wBBox.bottom)
		{
			wBBox.bottom = lpData[i].y;
		}
		if (lpData[i].y < wBBox.top)
		{
			wBBox.top = lpData[i].y;
		}
	}
	glbWindow(glbBlock, &wBBox);
	if (lpAxisOrg != NULL)   /* draw axes */
	{
		xt = MulDiv(lpAxisOrg->x - glbBlock->wOrg.x,
		            glbBlock->vExt.x, glbBlock->wExt.x) +
		     glbBlock->vOrg.x;
		yt = MulDiv(lpAxisOrg->y - glbBlock->wOrg.y,
		            glbBlock->vExt.y, glbBlock->wExt.y) +
		     glbBlock->vOrg.y;
		vBBox.left = MulDiv(wBBox.left - glbBlock->wOrg.x,
		                    glbBlock->vExt.x, glbBlock->wExt.x) +
		             glbBlock->vOrg.x;
		vBBox.right = MulDiv(wBBox.right - glbBlock->wOrg.x,
		                     glbBlock->vExt.x, glbBlock->wExt.x) +
		              glbBlock->vOrg.x;
		vBBox.top = MulDiv(wBBox.top - glbBlock->wOrg.x,
		                   glbBlock->vExt.x, glbBlock->wExt.x) +
		            glbBlock->vOrg.x;
		vBBox.bottom = MulDiv(wBBox.bottom - glbBlock->wOrg.x,
		                      glbBlock->vExt.x, glbBlock->wExt.x) +
		               glbBlock->vOrg.x;
		MoveToEx(hDC, vBBox.left, yt, NULL);
		LineTo(hDC, vBBox.right, yt);
		MoveToEx(hDC, vBBox.top, xt, NULL);
		LineTo(hDC, vBBox.bottom, xt);
	}
	for (i = 0; i < NumPoints; i++)
	{
		xt = MulDiv(lpData[i].x - glbBlock->wOrg.x,
		            glbBlock->vExt.x, glbBlock->wExt.x) +
		     glbBlock->vOrg.x;
		yt = MulDiv(lpData[i].y - glbBlock->wOrg.y,
		            glbBlock->vExt.y, glbBlock->wExt.y) +
		     glbBlock->vOrg.y;
		if (dxMark == 0 && dyMark == 0)
		{
			if (lpData[i].tag == TRUE)
			{
				LineTo(hDC, xt, yt);
			}
			else
			{
				MoveToEx(hDC, xt, yt, NULL);
			}
		}
		else
		{
			MoveToEx(hDC, xt - dxMark, yt, NULL);
			LineTo(hDC, xt + dxMark, yt);
			MoveToEx(hDC, xt, yt - dyMark, NULL);
			LineTo(hDC, xt, yt + dyMark);
		}
	}
} /* end of glbPlot */
