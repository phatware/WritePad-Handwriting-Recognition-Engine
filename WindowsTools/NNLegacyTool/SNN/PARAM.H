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

#ifndef     PARAM_INCLUDED
#define     PARAM_INCLUDED

#define     USE_RESAM_32     1

#define     USE_C_32         0
#define     USE_ASM_86       1
#define     USE_ASM_SH3      2
#define     USE_ASM_MIPS     3

#define     FIXED_ARITHMETIC    USE_C_32

#define    _POINT_DEFINED
typedef struct
{
	_SHORT  x;
	_SHORT  y;
} _POINT, _PTR  p_POINT;

#define    _3DPOINT_DEFINED
typedef struct
{
	_SHORT  x;
	_SHORT  y;
	_SHORT  z;
	_SHORT  p;
} _3DPOINT, _PTR  p_3DPOINT;

typedef struct
{
	_LONG    x;
	_LONG    y;
	_LONG   dx;
	_LONG   dy;
	_LONG    s;
	_LONG    r;
} _ODATA, _PTR  p_ODATA;

typedef struct
{
	_LONG    x;
	_LONG    y;
	_LONG    z;
	_LONG   dx;
	_LONG   dy;
	_LONG   dz;
	_LONG    s;
	_LONG    r;
} _ODATA3D, _PTR  p_ODATA3D;

typedef struct
{
	_LONG    Ax;
	_LONG    Ay;
	_LONG    Rx;
	_LONG    Ry;
	_LONG    s;
	_LONG    r;
} _ARDATA, _PTR  p_ARDATA;

typedef struct
{
	_LONG    Ax;
	_LONG    Ay;
	_LONG    Az;
	_LONG    Rx;
	_LONG    Ry;
	_LONG    Rz;
	_LONG    s;
	_LONG    r;
} _ARDATA3D, _PTR  p_ARDATA3D;


_ULONG  SQRT32(_ULONG);
// 2D
_VOID   ResetParam(_INT sm, p_ARDATA   pData, _LONG FullLen);
_LONG   ApprError(_INT sm, p_ARDATA   pARdata);
_VOID   Tracing(_INT sm, p_ARDATA   pData);
_LONG   Repar(_INT Sam, p_ODATA    pOdata,
              _INT ReSam, p_ARDATA   pARdata);
// 3D
_VOID   ResetParam3D(_INT sm, p_ARDATA3D pDdata, _LONG LenApp);
_LONG   ApprError3D(_INT sm, p_ARDATA3D pARdata);
_VOID   Tracing3D(_INT sm, p_ARDATA3D pData);
_LONG   Repar3D(_INT Sam, p_ODATA3D  pOdata,
                _INT ReSam, p_ARDATA3D pARdata);

_VOID            FDCT16(p_LONG pS);
_VOID            IDCT16(p_LONG pS);
#if  USE_RESAM_32
_VOID            FDCT32(p_LONG pS);
_VOID            IDCT32(p_LONG pS);
#endif

#endif  // PARAM_INCLUDED
