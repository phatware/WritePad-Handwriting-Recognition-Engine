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
#include <math.h>

typedef struct
{
	_UCHAR type;    //64
	_UCHAR size;    //16
	_UCHAR shift;   //16
	_UCHAR height;  //16
	_UCHAR orient;  //32
	_UCHAR penalty; //16 max value
	_UCHAR attrib;  // bit field
	_UCHAR emp;     // 2x4 point/dash
} 	XR_IMAGE;

typedef struct _LINE
{
	char x, y;
} LINE;

#define PI      3.14159265359
#define RAD  (180.0/PI)

#define TEAR_X    0
#define TAIL_L   64
#define TAIL_R   65

#define TAIL_L_MASK 1
#define TAIL_R_MASK 2

#define IsTailL(x)  ((x) & TAIL_L_MASK)
#define IsTailR(x)  ((x) & TAIL_R_MASK)

LINE   *ImgGetSample(int n, int *nPnt, int *iMid);
void    ImgDrwSample(HDC hDC, XR_IMAGE *XrImage, int x, int y, int dx, int dy);
_DOUBLE HSin(_SHORT);
_DOUBLE HCos(_SHORT);
_SHORT ImgSetWide(_SHORT dx, _SHORT dy);
_SHORT ImgGetWide(_SHORT wd, _SHORT dy);

#define IMG_DX	64
#define IMG_DY	64

_SHORT VxGetSize(_SHORT n);
_SHORT VxSetSize(_SHORT n);
_SHORT VxGetOrnt(_SHORT n);
_SHORT VxSetOrnt(_SHORT n);
_SHORT VxSetWide(_SHORT dx, _SHORT dy);
_SHORT VxGetWide(_SHORT wd, _SHORT dy);
_SHORT VxGetPosX(_SHORT dx);
_SHORT VxSetPosX(_SHORT dx);
_SHORT VxSetPosY(_SHORT y);
_SHORT VxGetPosY(_SHORT n);
