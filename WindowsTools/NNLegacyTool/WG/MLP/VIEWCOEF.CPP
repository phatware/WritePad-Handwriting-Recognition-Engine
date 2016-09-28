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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "ams_mg.h"

#include "snn.h"
#include "polyco.h"
#include "wggbl.h"
#include "zxlat.h"
#include "bitmapco.h"

#define  X_SIZE  200
#define  Y_SIZE  200

typedef struct
{
	_UCHAR syms[256];
	int    weights[256];
	int    points[2][32][3];
	_UCHAR map[GBM_NCOEFF];
} view_dialog_type, *p_view_dialog_type;


static HWND hViewDialogHandle = 0;
static view_dialog_type vd_data;

BOOL CALLBACK ViewDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

void IDCT32(long * dt);

/* **************************************************************************** */
/* *         Display coeefs and extra info in dilaog box                      * */
/* **************************************************************************** */
_INT  DrawCoeff(p_UCHAR syms, p_UCHAR w, p_UCHAR coeffs)
{
	//  RECT rect;

	if (hViewDialogHandle == 0)
	{
		HINSTANCE  hInst = GetModuleHandle(0);
		WNDCLASS  wc;


		wc.style = NULL;
		wc.lpfnWndProc = (WNDPROC) ViewDialogProc; /* Function to retrieve win messages  */
		wc.cbClsExtra = 0;                  /* No per-class extra data.           */
		wc.cbWndExtra = 0;                  /* No per-window extra data.          */
		wc.hInstance = hInst;          /* Application that owns the class.   */
		wc.hIcon = NULL;
		wc.hCursor = NULL;
		wc.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
		wc.lpszMenuName = NULL;               /* Name of menu resource in .RC file. */
		wc.lpszClassName = "ViewWinClass";     /* Name used in call to CreateWindow. */

		// if (!RegisterClass(&wc)) /*goto err*/;
		RegisterClass(&wc);


		hViewDialogHandle = CreateWindow((LPSTR)"ViewWinClass",
		                                 (LPSTR)"MLP View Window",
		                                 WS_BORDER |
		                                 WS_CAPTION |
		                                 WS_POPUP |
		                                 WS_SYSMENU |
		                                 WS_VISIBLE,
		                                 0,
		                                 20,
		                                 X_SIZE + GetSystemMetrics(SM_CXBORDER) * 2,
		                                 Y_SIZE + GetSystemMetrics(SM_CYBORDER) * 2
		                                 + GetSystemMetrics(SM_CYCAPTION),
		                                 hMainWnd,
		                                 NULL,
		                                 hInst,
		                                 (LPSTR) NULL
		                                );

		if (hViewDialogHandle)
		{
			memset((void*) &vd_data, 0, sizeof(vd_data));
			ShowWindow(hViewDialogHandle, SW_SHOWNORMAL);       /* Show the window                        */
			UpdateWindow(hViewDialogHandle);               /* Sends WM_PAINT message                 */
		}
	}

	if (hViewDialogHandle)
	{
		int    i;
		long   dt[32];
		int    all_sorted;
		_UCHAR symTmp;
		int    wTmp;

		memset(&vd_data, 0, sizeof(vd_data));

		for (i = 32; i < 256; i++)
		{
			vd_data.syms[i] = (_UCHAR) i;
			vd_data.weights[i] = w[i];
		}

		//Sort according to answer weights:

		all_sorted = 0;
		while (!all_sorted)
		{
			all_sorted = 1;
			for (i = 1; i < 256; i++)
			{
				if (vd_data.weights[i] > vd_data.weights[i - 1])
				{
					wTmp = vd_data.weights[i];
					vd_data.weights[i] = vd_data.weights[i - 1];
					vd_data.weights[i - 1] = wTmp;

					symTmp = vd_data.syms[i];
					vd_data.syms[i] = vd_data.syms[i - 1];
					vd_data.syms[i - 1] = symTmp;

					all_sorted = 0;
				}
			}
		}


		//Prepare trajectory data:
		memset(dt, 0, sizeof(dt));
		for (i = 0; i < PC_N_INT_COEFFS - 1; i++)
		{
			dt[i + 1] = (int) 100 * (coeffs[(i) * 3 + 6] - 128);
		}
		IDCT32(dt);
		for (i = 0; i < 32; i++)
		{
			vd_data.points[1][i][0] = (int) dt[i];
		}

		memset(dt, 0, sizeof(dt));
		for (i = 0; i < PC_N_INT_COEFFS - 1; i++)
		{
			dt[i + 1] = 100 * (coeffs[(i) * 3 + 5] - 128);
		}
		IDCT32(dt);
		for (i = 0; i < 32; i++)
		{
			vd_data.points[1][i][1] = (int) dt[i];
		}

		memset(dt, 0, sizeof(dt));
		for (i = 0; i < PC_N_INT_COEFFS - 1; i++)
		{
			dt[i + 1] = 100 * (coeffs[(i) * 3 + 4] - 128);
		}
		IDCT32(dt);
		for (i = 0; i < 32; i++)
		{
			vd_data.points[1][i][2] = (int) dt[i];
		}

		memcpy(vd_data.map, coeffs + PC_NUM_COEFF, sizeof(vd_data.map));

		InvalidateRect(hViewDialogHandle, 0, TRUE);
		UpdateWindow(hViewDialogHandle);
		Sleep(0);
	}

	return 0;
}

/* ************************************************************************** */
/* *  Proc for dialog box for temp status display                           * */
/* ************************************************************************** */
BOOL CALLBACK ViewDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message)
	{
		case WM_PAINT:
		{
			int i, j;
			char str[MLP_NET_NUMOUTPUTS * 5];
			HDC  hdc;
			RECT rect;
			SIZE ts;
			PAINTSTRUCT PaintStruct;
			HPEN hPen, hPrevPen;
			int  x1, y1;

			hdc = BeginPaint(hDlg, &PaintStruct);

			SetBkMode(hdc, TRANSPARENT);

			//Make window borders visible:
			GetClientRect(hDlg, &rect);

			hPen = CreatePen(PS_SOLID, 3, RGB(128, 128, 128));
			hPrevPen = (HPEN) SelectObject(hdc, hPen);

			MoveToEx(hdc, 0, 0, NULL);
			LineTo(hdc, 0, rect.bottom - 1);
			LineTo(hdc, rect.right - 1, rect.bottom - 1);
			LineTo(hdc, rect.right - 1, 0);
			LineTo(hdc, 0, 0);

			SelectObject(hdc, hPrevPen);
			DeleteObject(hPen);

			// --------------- Draw bitmap ---------------------
			{
				int n;
				int x, y, qx, qy, nq, c;

				for (n = 0; n < GBM_NCOEFF; n++)
				{
					nq = n / 16;
					qx = (nq%GBM_QSX)*(GBM_XSIZE / GBM_QSX);
					qy = (nq / GBM_QSX)*(GBM_YSIZE / GBM_QSY);
					x = (n - nq * 16) % 4;
					y = (n - nq * 16) / 4;
					c = RGB(0, vd_data.map[n] + 63, vd_data.map[n] + 63);

					SetPixel(hdc, 4 * (qx + x) + 4, 4 * (qy + y) + 16, c);
					SetPixel(hdc, 4 * (qx + x) + 4 + 1, 4 * (qy + y) + 16, c);
					SetPixel(hdc, 4 * (qx + x) + 4, 4 * (qy + y) + 16 + 1, c);
					SetPixel(hdc, 4 * (qx + x) + 4 + 1, 4 * (qy + y) + 16 + 1, c);
				}
			}

			hPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
			hPrevPen = (HPEN) SelectObject(hdc, hPen);

			MoveToEx(hdc, 3, 15, NULL);
			LineTo(hdc, 4 + GBM_XSIZE * 4, 15);
			LineTo(hdc, 4 + GBM_XSIZE * 4, 16 + GBM_YSIZE * 4);
			LineTo(hdc, 3, 16 + GBM_YSIZE * 4);
			LineTo(hdc, 3, 15);

			SelectObject(hdc, hPrevPen);
			DeleteObject(hPen);

			// --------------- Draw ink ------------------------

			SetTextColor(hdc, RGB(64, 255, 64));

			str[0] = 0;
			ts.cx = ts.cy = 0;
			for (i = j = 0; i < MLP_NET_NUMOUTPUTS && vd_data.syms[i] != 0; i++)
			{
				//        j += sprintf(str+j, "%c%d ", MACToANSI(vd_data.syms[i]), (_INT)(100*vd_data.weights[i]/256));
				j += sprintf(str + j, "%c%d ", vd_data.syms[i], (_INT) (100 * vd_data.weights[i] / 256));
				GetTextExtentPoint32(hdc, str, j, &ts);
				if (ts.cx > rect.right - 20)
				{
					break;
				}
			}

			TextOut(hdc, 4, rect.bottom - 3 - ts.cy, str, strlen(str));

			//      hPen     = CreatePen(PS_SOLID, 1, RGB(0, 128, 128));

			x1 = 90 + vd_data.points[1][0][0] / 200;
			y1 = 90 + vd_data.points[1][0][1] / 200;

			Rectangle(hdc, x1 - 1, y1 - 1, x1 + 2, y1 + 2);

			MoveToEx(hdc, x1, y1, 0);
			for (i = 0; i < 32; i++)
			{
				int n = vd_data.points[1][i][2] / 50 + 128;
				if (n < 0)
				{
					n = 0;
				}
				if (n > 255)
				{
					n = 255;
				}
				int m = 255 - n + 128;
				if (m < 0)
				{
					m = 0;
				}
				if (m > 255)
				{
					m = 255;
				}
				hPen = CreatePen(PS_SOLID, 1, RGB(255, m, n));
				hPrevPen = (HPEN) SelectObject(hdc, hPen);
				LineTo(hdc, 90 + vd_data.points[1][i][0] / 200, 90 + vd_data.points[1][i][1] / 200);
				SelectObject(hdc, hPrevPen);
				DeleteObject(hPen);
			}


			EndPaint(hDlg, &PaintStruct);
			return (TRUE);
		}

		case WM_DESTROY:
			hViewDialogHandle = 0;
			break; //to DefWindowProc
	}


	return DefWindowProc(hDlg, message, wParam, lParam);
}
