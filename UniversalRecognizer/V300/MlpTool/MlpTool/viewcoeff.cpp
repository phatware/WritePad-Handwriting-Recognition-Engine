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

#undef LANG_DUTCH
#undef LANG_ENGLISH
#undef LANG_FINNISH
#undef LANG_GERMAN
#undef LANG_ITALIAN
#undef LANG_PORTUGUESE
#undef LANG_SPANISH 
#undef LANG_FRENCH

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <commdlg.h>
#include <conio.h>

#pragma hdrstop

#include <iostream>

#include "mlp.h"
#include "main.h"


static HWND hViewDialogHandle = 0;
static view_dialog_type vd_data;

BOOL CALLBACK ViewDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

extern _VOID  IDCT32(p_LONG  x);

/* **************************************************************************** */
/* *         Display coeefs and extra info in dilaog box                      * */
/* **************************************************************************** */

int  DrawCoeff(uchar rsym, float w, int num, samples_storage_class * samples, uchar * coeffs)
{
    //  RECT rect;
    
    if (hViewDialogHandle == 0)
    {
        HMODULE  hInst = GetModuleHandle( NULL );
        WNDCLASSA  wc;

        wc.style         = NULL;
        wc.lpfnWndProc   = (WNDPROC)ViewDialogProc; /* Function to retrieve win messages  */
        wc.cbClsExtra    = 0;                  /* No per-class extra data.           */
        wc.cbWndExtra    = 0;                  /* No per-window extra data.          */
        wc.hInstance     = hInst;          /* Application that owns the class.   */
        wc.hIcon         = NULL;
        wc.hCursor       = NULL;
        wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
        wc.lpszMenuName  = NULL;               /* Name of menu resource in .RC file. */
        wc.lpszClassName = "ViewWinClass";     /* Name used in call to CreateWindow. */
        
        // if (!RegisterClass(&wc)) /*goto err*/;
        RegisterClassA( &wc );
        
        
        hViewDialogHandle = CreateWindowA((LPCSTR)"ViewWinClass",
                                         (LPCSTR)"MLP View Window",
                                         WS_BORDER       |
                                         WS_CAPTION      |
                                         //                           WS_CLIPCHILDREN |
                                         //                           WS_MINIMIZEBOX  |
                                         WS_POPUP        |
                                         //                           WS_THICKFRAME   |
                                         //                           WS_SYSMENU      |
                                         WS_VISIBLE,
                                         0,  //CW_USEDEFAULT,
                                         20, //CW_USEDEFAULT,
                                         200,
                                         200,
                                         NULL,
                                         NULL,
                                         (HINSTANCE)hInst,
                                         NULL
                                         );
        
        if (hViewDialogHandle)
        {
            memset((void*)&vd_data, 0, sizeof(vd_data));
            ShowWindow(hViewDialogHandle, SW_SHOWNORMAL);       /* Show the window                        */
            UpdateWindow(hViewDialogHandle);               /* Sends WM_PAINT message                 */
        }
    }
    
    if (hViewDialogHandle)
    {
        int  i;
        _LONG dt[32];
        p_sample_type samp;
        
        samples->GetSample(num, &samp);
        
        vd_data.ns   = num;
        vd_data.nv   = samp->nvs[0] >> 4;
        vd_data.rsym = rsym;
        vd_data.w    = w;
        //    for (i = 0, vd_data.rsym = 0; i < 8; i ++) vd_data.rsym |= (uchar)((coeffs[i+MAIN_SIZE_SAMPLE] > 0.5) ? (0x01 << i) : 0);
        
        for (i = 0; i < MAIN_NUMSYMINSAMP; i ++)
            vd_data.syms[i] = samp->syms[i];
        
        memset(dt, 0, sizeof(dt));
        for (i = 0; i < 9; i ++)
            dt[i+1] = 100*(samp->data[(i)*3+6]-128);
        IDCT32(dt);
        for (i = 0; i < 32; i ++)
            vd_data.points[0][i][0] = (int)dt[i];
        
        memset(dt, 0, sizeof(dt));
        for (i = 0; i < 9; i ++)
            dt[i+1] = 100*(samp->data[(i)*3+5]-128);
        IDCT32(dt);
        for (i = 0; i < 32; i ++)
            vd_data.points[0][i][1] = (int)dt[i];
        
        memset(dt, 0, sizeof(dt));
        for (i = 0; i < 9; i ++)
            dt[i+1] = 100*(samp->data[(i)*3+4]-128);
        IDCT32(dt);
        for (i = 0; i < 32; i ++)
            vd_data.points[0][i][2] = (int)dt[i];
        
        memset(dt, 0, sizeof(dt));
        for (i = 0; i < 9; i ++)
            dt[i+1] = (int)100*(coeffs[(i)*3+6]-128);
        IDCT32(dt);
        for (i = 0; i < 32; i ++)
            vd_data.points[1][i][0] = (int)dt[i];
        
        memset(dt, 0, sizeof(dt));
        for (i = 0; i < 9; i ++)
            dt[i+1] = 100*(coeffs[(i)*3+5]-128);
        IDCT32(dt);
        for (i = 0; i < 32; i ++)
            vd_data.points[1][i][1] = (int)dt[i];
        
        memset(dt, 0, sizeof(dt));
        for (i = 0; i < 9; i ++)
            dt[i+1] = 100*(coeffs[(i)*3+4]-128);
        IDCT32(dt);
        for (i = 0; i < 32; i ++)
            vd_data.points[1][i][2] = (int)dt[i];
        
        memcpy(vd_data.bmap, coeffs+MAIN_SIZE_SAMPLE, sizeof(vd_data.bmap));
        
        InvalidateRect(hViewDialogHandle, 0, TRUE);
        UpdateWindow(hViewDialogHandle);
        PS_Yield();
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
#if 0
        case WM_ERASEBKGND:
        {
            HDC hDC = (HDC)wParam;
            RECT rect;
            HBRUSH hOldBrush, hNewBrush;
            
            //      hDC = GetDC(hDlg);
            hNewBrush = CreateSolidBrush(RGB(0,0,0));
            hOldBrush = SelectObject(hDC, hNewBrush);
            GetClientRect(hDlg, &rect);
            Rectangle(hDC, rect.left, rect.top, rect.right, rect.bottom);
            SelectObject(hDC, hOldBrush);
            DeleteObject(hNewBrush);
            //      ReleaseDC(hDlg, hDC);
            
            return (TRUE);
        }
#endif
            
        case WM_PAINT:
        {
            int i;
            HDC hdc;
            PAINTSTRUCT PaintStruct;
            char str[64];
            char str1[MAIN_NUMSYMINSAMP+4];
            HPEN hPen, hPrevPen;
            
            hdc = BeginPaint(hDlg, &PaintStruct);
            
            SetBkMode(hdc, TRANSPARENT);
            
            
            // --------------- Draw bitmap ---------------------
            {
                int n;
                int x, y, qx, qy, nq, c;
                
                for (n = 0; n < GBM_NCOEFF; n++)
                {
                    nq = n/16;
                    qx = (nq%GBM_QSX)*(GBM_XSIZE/GBM_QSX);
                    qy = (nq/GBM_QSX)*(GBM_YSIZE/GBM_QSY);
                    x  = (n - nq*16) % 4;
                    y  = (n - nq*16) / 4;
                    c  = RGB(0,vd_data.bmap[n]+63,vd_data.bmap[n]+63);
                    
                    SetPixel(hdc, 4*(qx+x)+4,   4*(qy+y)+16, c);
                    SetPixel(hdc, 4*(qx+x)+4+1, 4*(qy+y)+16, c);
                    SetPixel(hdc, 4*(qx+x)+4,   4*(qy+y)+16+1, c);
                    SetPixel(hdc, 4*(qx+x)+4+1, 4*(qy+y)+16+1, c);
                }
            }
            
            
            hPen     = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
            hPrevPen = (HPEN)SelectObject(hdc, hPen);
            
            MoveToEx(hdc, 3, 15, NULL);
            LineTo(hdc, 4+GBM_XSIZE*4, 15);
            LineTo(hdc, 4+GBM_XSIZE*4, 16+GBM_YSIZE*4);
            LineTo(hdc, 3, 16+GBM_YSIZE*4);
            LineTo(hdc, 3, 15);
            
            SelectObject(hdc, hPrevPen);
            DeleteObject(hPen);
            
            // --------------- Draw ink ------------------------
            
            MoveToEx(hdc, 90+vd_data.points[1][0][0]/200, 80+vd_data.points[1][0][1]/200, 0);
            for (i = 0; i < 32; i ++)
            {
                int n =  vd_data.points[1][i][2]/50+128; if (n < 0) n = 0; if (n > 255) n = 255;
                hPen     = CreatePen(PS_SOLID, 1, RGB(0, 192-n/2, n/2));
                hPrevPen = (HPEN)SelectObject(hdc, hPen);
                LineTo(hdc, 90+vd_data.points[1][i][0]/200, 80+vd_data.points[1][i][1]/200);
                SelectObject(hdc, hPrevPen);
                DeleteObject(hPen);
            }
            
            MoveToEx(hdc, 90+vd_data.points[0][0][0]/200, 80+vd_data.points[0][0][1]/200, 0);
            for (i = 0; i < 32; i ++)
            {
                int n =  vd_data.points[0][i][2]/50+128; if (n < 0) n = 0; if (n > 255) n = 255;
                int m = 255 - n + 128; if (m < 0) m = 0; if (m > 255) m = 255;
                hPen     = CreatePen(PS_SOLID, 1, RGB(255, m, n));
                hPrevPen = (HPEN)SelectObject(hdc, hPen);
                LineTo(hdc, 90+vd_data.points[0][i][0]/200, 80+vd_data.points[0][i][1]/200);
                SelectObject(hdc, hPrevPen);
                DeleteObject(hPen);
            }
            
            SetTextColor(hdc, RGB(0, 255, 0));
            
            sprintf(str, "Samp %d, Nv %d", vd_data.ns, vd_data.nv);
            TextOutA(hdc, 4, 0, str, strlen(str));
            
            strncpy(str1, (char*)&vd_data.syms[0], MAIN_NUMSYMINSAMP); str1[MAIN_NUMSYMINSAMP]  = 0;
            if (str1[1]) sprintf(str, "This is ' %c ', not ' %s '.", vd_data.syms[0], &str1[1]);
            else sprintf(str, "This is ' %c '.", vd_data.syms[0]);
            TextOutA(hdc, 4, 162, str, strlen(str));
            
            SetTextColor(hdc, RGB(255, 0, 0));
            
            sprintf(str, "Rsym %c, W: %f", vd_data.rsym, vd_data.w);
            TextOutA(hdc, 4, 146, str, strlen(str));
            
            EndPaint(hDlg, &PaintStruct);
            return (TRUE);
        }
    }    
    return DefWindowProc(hDlg, message, wParam, lParam);
}

