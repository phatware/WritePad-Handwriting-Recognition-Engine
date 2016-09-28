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

#include    "bastypes.h"

#define    _2C1_8_H      0x8A
#define    _2C1_8_L      0x8B
#define      C2_8_H      0xB5
#define      C2_8_L      0x04
#define    _2C3_8_H      0x14E
#define    _2C3_8_L      0x7A

#define    _2C01_16_H    0x82
#define    _2C01_16_L    0x81
#define    _2C03_16_H    0x99
#define    _2C03_16_L    0xF1
#define    _2C05_16_H    0xE6
#define    _2C05_16_L    0x64
#define    _2C07_16_H    0x290
#define    _2C07_16_L    0x1B

#define    _2C01_32_H    0x80
#define    _2C01_32_L    0x9E
#define    _2C03_32_H    0x85
#define    _2C03_32_L    0x2C
#define    _2C05_32_H    0x91
#define    _2C05_32_L    0x23
#define    _2C07_32_H    0xA5
#define    _2C07_32_L    0x96
#define    _2C09_32_H    0xC9
#define    _2C09_32_L    0xC4
#define    _2C11_32_H    0x10F
#define    _2C11_32_L    0x88
#define    _2C13_32_H    0x1B8
#define    _2C13_32_L    0xF2
#define    _2C15_32_H    0x519
#define    _2C15_32_L    0xE4

#define    _2C01_64_H    0x80
#define    _2C01_64_L    0x27
#define    _2C03_64_H    0x81
#define    _2C03_64_L    0x66
#define    _2C05_64_H    0x83
#define    _2C05_64_L    0xf4
#define    _2C07_64_H    0x87
#define    _2C07_64_L    0xf2
#define    _2C09_64_H    0x8d
#define    _2C09_64_L    0x98
#define    _2C11_64_H    0x95
#define    _2C11_64_L    0x3b
#define    _2C13_64_H    0x9f
#define    _2C13_64_L    0x5c
#define    _2C15_64_H    0xac
#define    _2C15_64_L    0xc0
#define    _2C17_64_H    0xbe
#define    _2C17_64_L    0x99
#define    _2C19_64_H    0xd6
#define    _2C19_64_L    0xdf
#define    _2C21_64_H    0xf8
#define    _2C21_64_L    0xfa
#define    _2C23_64_H    0x12b
#define    _2C23_64_L    0x60
#define    _2C25_64_H    0x17b
#define    _2C25_64_L    0xf2
#define    _2C27_64_H    0x20e
#define    _2C27_64_L    0xca
#define    _2C29_64_H    0x368
#define    _2C29_64_L    0x59
#define    _2C31_64_H    0xa30
#define    _2C31_64_L    0xa4

ROM_DATA_EXTERNAL _LONG   _2C16 [];

ROM_DATA_EXTERNAL _LONG   _2C32 [];

/************************************************************************/

_VOID  FDCT4(p_LONG  x);
_VOID  IDCT4(p_LONG  x);
_VOID  FDCT8(p_LONG  x);
_VOID  IDCT8(p_LONG  x);
_VOID  FDCT16(p_LONG  x);
_VOID  IDCT16(p_LONG  x);
_VOID  FDCT32(p_LONG  x);
_VOID  IDCT32(p_LONG  x);

/************************************************************************/
/*         4-point Forward Discrete Cosine Transform                    */
/************************************************************************/

_VOID  FDCT4(p_LONG  x)
{
	_LONG  x0, x1, x2, x3, a0;

	//  Stage  0, reordering
	x0 = x[0];
	x1 = x[1];
	x2 = x[3];
	x3 = x[2];
	//  Stage  1, Butterflys and Rotations
	a0 = x0 + x2;
	x2 = x0 - x2;
	x0 = a0;
	a0 = x1 + x3;
	x3 = x1 - x3;
	x1 = a0;
	x2 = ((x2 * _2C1_8_H) >> 8) + ((x2 * _2C1_8_L) >> 16);
	x3 = ((x3 * _2C3_8_H) >> 8) + ((x3 * _2C3_8_L) >> 16);
	//  Stage 2,  Butterfly and Rotation
	a0 = x0 + x1;
	x1 = x0 - x1;
	x0 = a0;
	a0 = x2 + x3;
	x3 = x2 - x3;
	x2 = a0;
	x1 = ((x1 * C2_8_H) >> 8) + ((x1 * C2_8_L) >> 16);
	x3 = ((x3 * C2_8_H) >> 8) + ((x3 * C2_8_L) >> 16);
	//  Stage  3, back reordering
	x[0] = x0;
	x[1] = x2 + x3;
	x[2] = x1;
	x[3] = x3;
}  //  End of FDCT4


/************************************************************************/
/*         4-point Inverse Discrete Cosine Transform                    */
/************************************************************************/

_VOID  IDCT4(p_LONG  x)
{
	_LONG  x0, x1, x2, x3, a0;

	//  Stage  0, reordering
	x0 = x[0];
	x1 = x[2];
	x2 = x[1];
	x3 = x[3];

	//  Stage  1, additional summarization
	x3 += x2;

	//  Stage  2,
	a0 = (x1 * C2_8_H);
	a0 >>= 8;
	x1 = (x1 * C2_8_L);
	x1 >>= 16;
	x1 += a0;
	a0 = (x3 * C2_8_H);
	a0 >>= 8;
	x3 = (x3 * C2_8_L);
	x3 >>= 16;
	x3 += a0;

	//  Stage  3, Butterflys
	a0 = x0;
	x0 += x1;
	a0 -= x1;
	x1 = a0;

	a0 = x2;
	x2 += x3;
	a0 -= x3;
	x3 = a0;

	//  Stage 4,
	a0 = (x2 * _2C1_8_H);
	a0 >>= 8;
	x2 = (x2 * _2C1_8_L);
	x2 >>= 16;
	x2 += a0;
	a0 = (x3 * _2C3_8_H);
	a0 >>= 8;
	x3 = (x3 * _2C3_8_L);
	x3 >>= 16;
	x3 += a0;

	//  Stage 5,  Butterfly
	a0 = x0;
	x0 += x2;
	a0 -= x2;
	x2 = a0;

	a0 = x1;
	x1 += x3;
	a0 -= x3;
	x3 = a0;

	//  Stage 6,  back reordering
	x[0] = x0;
	x[1] = x1;
	x[2] = x3;
	x[3] = x2;
} //  End of IDCT4


/************************************************************************/
/*    8 - Point  Forward  Discrete  Cosine  Transform                   */
/************************************************************************/

_VOID   FDCT8(p_LONG  x)
{
	_LONG  x0, x1, x2, x3, x4;

	//  Stage  1, reordering, butterflys and rotations
	x1 = x0 = x[0];
	x4 = x[7];
	x0 += x4;       //  x0 = x[0] + x[7];
	x1 -= x4;       //  x1 = x[0] - x[7];
	x2 = x3 = x[3];
	x4 = x[4];
	x2 += x4;       //  x2 = x[3] + x[4];
	x3 -= x4;       //  x3 = x[3] - x[4];
	x[0] = x0;
	x[3] = x2;
	x0 = ((x1 * _2C01_16_H) >> 8);
	x0 += ((x1 * _2C01_16_L) >> 16);
	x[4] = x0;
	x0 = ((x3 * _2C07_16_H) >> 8);
	x0 += ((x3 * _2C07_16_L) >> 16);
	x[7] = x0;

	x1 = x0 = x[1];
	x4 = x[6];
	x0 += x4;       //  x0 = x[1] + x[6];
	x1 -= x4;       //  x1 = x[1] - x[6];
	x2 = x3 = x[2];
	x4 = x[5];
	x2 += x4;       //  x2 = x[2] + x[5];
	x3 -= x4;       //  x3 = x[2] - x[5];
	x[1] = x0;
	x[2] = x2;
	x0 = ((x1 * _2C03_16_H) >> 8);
	x0 += ((x1 * _2C03_16_L) >> 16);
	x[5] = x0;
	x0 = ((x3 * _2C05_16_H) >> 8);
	x0 += ((x3 * _2C05_16_L) >> 16);
	x[6] = x0;

	//  Stage  2, 2 x four point FDCT
	FDCT4(x);
	FDCT4(x + 4);

	//  Stage  3, Second reordering
	x0 = x[7];
	x1 = x[6];
	x[6] = x[3];     //  x[6] = x'[3]
	x0 += x1;     //  x0   = x'[6] + x'[7];
	x2 = x[5];
	x[5] = x0;     //  x[5] = x'[6] + x'[7];
	x0 = x[4];     //
	x[4] = x[2];     //  x[4] = x'[2];
	x1 += x2;     //  x1   = x'[5] + x'[6];
	x[3] = x1;     //  x[3] = x'[5] + x'[6];
	x[2] = x[1];     //  x[2] = x'[1];
	x0 += x2;     //  x0   = x'[4] + x'[5];
	x[1] = x0;     //  x[1] = x'[4] + x'[5];
}  // End of  FDCT8 ( p_LONG  x )

/************************************************************************/
/*    8 - Point  Inverse  Discrete  Cosine  Transform                   */
/************************************************************************/

_VOID   IDCT8(p_LONG  x)
{
	//  Stage  1, reordering
	{
		_LONG  x0, x1;
		x0 = x[1];
		x1 = x[4];
		x[4] = x0;
		x0 = x[2];
		x[2] = x1;
		x[1] = x0;
		x0 = x[6];
		x1 = x[3];
		x[3] = x0;
		x0 = x[5];
		x[5] = x1;
		x[6] = x0;
	}
	//  Stage  2, Additional summarisation
	{
		_LONG  x0, x1;
		x0 = x[7];
		x1 = x[6];
		x0 += x1;
		x[7] = x0;
		x0 = x[5];
		x1 += x0;
		x[6] = x1;
		x1 = x[4];
		x0 += x1;
		x[5] = x0;
	}
	//  Stage  3, 2 x four point IDCT
	IDCT4(x);
	IDCT4(x + 4);

	//  Stage  4, Rotation
	{
		_LONG  a0, x0;

		x0 = x[4];
		a0 = (x0 * _2C01_16_H);
		a0 >>= 8;
		x0 = (x0 * _2C01_16_L);
		x0 >>= 16;
		x0 += a0;
		x[4] = x0;

		x0 = x[5];
		a0 = (x0 * _2C03_16_H);
		a0 >>= 8;
		x0 = (x0 * _2C03_16_L);
		x0 >>= 16;
		x0 += a0;
		x[5] = x0;

		x0 = x[6];
		a0 = (x0 * _2C05_16_H);
		a0 >>= 8;
		x0 = (x0 * _2C05_16_L);
		x0 >>= 16;
		x0 += a0;
		x[6] = x0;

		x0 = x[7];
		a0 = (x0 * _2C07_16_H);
		a0 >>= 8;
		x0 = (x0 * _2C07_16_L);
		x0 >>= 16;
		x0 += a0;
		x[7] = x0;
	}

	// Stage  5, Butterfly
	{
		_LONG  x0, x1, x2, x3;
		x0 = x[0];
		x2 = x0;
		x1 = x[4];
		x0 += x1;
		x[0] = x0;
		x2 -= x1;
		x0 = x[3];
		x3 = x0;
		x1 = x[7];
		x0 += x1;
		x[3] = x0;
		x3 -= x1;
		x[4] = x3;
		x[7] = x2;

		x0 = x[1];
		x2 = x0;
		x1 = x[5];
		x0 += x1;
		x[1] = x0;
		x2 -= x1;
		x0 = x[2];
		x3 = x0;
		x1 = x[6];
		x0 += x1;
		x[2] = x0;
		x3 -= x1;
		x[5] = x3;
		x[6] = x2;
	}

}  // End of  IDCT8 ( p_LONG  x )



/************************************************************************/
/*   16 - Point  Forward  Discrete  Cosine  Transform                   */
/************************************************************************/

_VOID   FDCT16(p_LONG  x)
{
	//  Stage  1, reordering, butterflys and rotations
	{
		_LONG  x0, x1, x2, x3, x4;
		x1 = x0 = x[0];
		x4 = x[15];
		x0 += x4;       //  x0 = x[0] + x[15];
		x1 -= x4;       //  x1 = x[0] - x[15];
		x2 = x3 = x[7];
		x4 = x[8];
		x2 += x4;       //  x2 = x[7] + x[8];
		x3 -= x4;       //  x3 = x[7] - x[8];
		x[0] = x0;
		x[7] = x2;
		x0 = ((x1 * _2C01_32_H) >> 8);
		x0 += ((x1 * _2C01_32_L) >> 16);
		x[8] = x0;
		x0 = ((x3 * _2C15_32_H) >> 8);
		x0 += ((x3 * _2C15_32_L) >> 16);
		x[15] = x0;

		x1 = x0 = x[1];
		x4 = x[14];
		x0 += x4;       //  x0 = x[1] + x[14];
		x1 -= x4;       //  x1 = x[1] - x[14];
		x2 = x3 = x[6];
		x4 = x[9];
		x2 += x4;       //  x2 = x[6] + x[9];
		x3 -= x4;       //  x3 = x[6] - x[9];
		x[1] = x0;
		x[6] = x2;
		x0 = ((x1 * _2C03_32_H) >> 8);
		x0 += ((x1 * _2C03_32_L) >> 16);
		x[9] = x0;
		x0 = ((x3 * _2C13_32_H) >> 8);
		x0 += ((x3 * _2C13_32_L) >> 16);
		x[14] = x0;

		x1 = x0 = x[2];
		x4 = x[13];
		x0 += x4;       //  x0 = x[2] + x[13];
		x1 -= x4;       //  x1 = x[2] - x[13];
		x2 = x3 = x[5];
		x4 = x[10];
		x2 += x4;       //  x2 = x[5] + x[10];
		x3 -= x4;       //  x3 = x[5] - x[10];
		x[2] = x0;
		x[5] = x2;
		x0 = ((x1 * _2C05_32_H) >> 8);
		x0 += ((x1 * _2C05_32_L) >> 16);
		x[10] = x0;
		x0 = ((x3 * _2C11_32_H) >> 8);
		x0 += ((x3 * _2C11_32_L) >> 16);
		x[13] = x0;

		x1 = x0 = x[3];
		x4 = x[12];
		x0 += x4;       //  x0 = x[3] + x[12];
		x1 -= x4;       //  x1 = x[3] - x[12];
		x2 = x3 = x[4];
		x4 = x[11];
		x2 += x4;       //  x2 = x[4] + x[11];
		x3 -= x4;       //  x3 = x[4] - x[11];
		x[3] = x0;
		x[4] = x2;
		x0 = ((x1 * _2C07_32_H) >> 8);
		x0 += ((x1 * _2C07_32_L) >> 16);
		x[11] = x0;
		x0 = ((x3 * _2C09_32_H) >> 8);
		x0 += ((x3 * _2C09_32_L) >> 16);
		x[12] = x0;
	}  // End of stage  1;

	//  Stage  2, 2 x 8-point FDCT
	FDCT8(x);
	FDCT8(x + 8);

	//  Stage  3, additional summarization
	{
		_LONG  x0, x1;
		_INT   x2;
		p_LONG  pX;

		pX = x + 8;
		x0 = *pX;
		x2 = 7;
		do
		{
			x1 = *(pX + 1);
			x0 += x1;
			*pX = x0;
			x0 = x1;
			pX++;
			x2--;
		}
		while (x2 > 0);
	}  //  End of Stage 3;

	//  Stage  4, second reordering
	{
		_LONG  x0, x1;
		_INT   i0, i1;
		for (i0 = 1; i0 < 8; i0 += 2)
		{
			i1 = i0;
			x1 = x[i1];
			do
			{
				i1 <<= 1;
				if (i1 > 15)
				{
					i1 -= 15;
				}
				x0 = x[i1];
				x[i1] = x1;
				x1 = x0;
			}
			while (i1 != i0);
		}
	} //  End of Stage 4;

} // End of FDCT16 (p_LONG  x);



/************************************************************************/
/*   16 - Point  Inverse  Discrete  Cosine  Transform                   */
/************************************************************************/

_VOID   IDCT16(p_LONG  x)
{
	//  Stage  1, second reordering
	{
		_LONG  x0, x1;
		_INT   i0, i1, i2;
		for (i2 = 1; i2 < 8; i2 += 2)
		{
			i1 = i2;
			x0 = x[i1];
			do
			{
				i0 = i1;
				i1 <<= 1;
				if (i1 > 15)
				{
					i1 -= 15;
				}
				x1 = x[i1];
				x[i0] = x1;
			}
			while (i1 != i2);
			x[i0] = x0;
		}
	} //  End of Stage 1;

	//  Stage  2, Additional summarisation
	{
		_LONG  x0, x1;
		_INT   x2;
		p_LONG  pX;

		pX = x + 15;
		x0 = *pX;
		x2 = 7;
		do
		{
			x1 = *(pX - 1);
			x0 += x1;
			*pX = x0;
			x0 = x1;
			pX--;
			x2--;
		}
		while (x2 > 0);
	}  //  End of Stage 3;

	//  Stage  3, 2 x four point IDCT
	IDCT8(x);
	IDCT8(x + 8);

	//  Stage  4, Rotation and Butterfly
	{
		_LONG  x0, x1, x2;
		_INT   i0;
		i0 = 0;
		do
		{
			x2 = x[i0 + 8];
			x0 = _2C16[i0];
			x1 = x0;
			x0 >>= 8;
			x1 &= 0xFF;
			x0 = ((x2 * x0) >> 8);
			x1 = ((x2 * x1) >> 16);
			x0 += x1;    //  x0 = x2 * _2C01_16
			x1 = x[i0];
			x2 = x1;
			x1 += x0;
			x2 -= x0;
			x[i0] = x1;
			x[i0 + 8] = x2;
			i0++;
		}
		while (i0 < 8);
	}

	//  Stage  5, reordering
	{
		_LONG  x0, x1;
		_INT   i0, i1;
		i0 = 8;
		i1 = 15;
		do
		{
			x0 = x[i0];
			x1 = x[i1];
			x[i0] = x1;
			x[i1] = x0;
			i0++;
			i1--;
		}
		while (i0 < i1);
	}
} // End of IDCT16 (p_LONG  x);

/************************************************************************/
/*   32 - Point  Forward  Discrete  Cosine  Transform                   */
/************************************************************************/

_VOID   FDCT32(p_LONG  x)
{
	//  Stage  1, reordering
	{
		_LONG  x0, x1;
		_INT   i0, i1;
		i0 = 16;
		i1 = 31;
		do
		{
			x0 = x[i0];
			x1 = x[i1];
			x[i0] = x1;
			x[i1] = x0;
			i0++;
			i1--;
		}
		while (i0 < i1);
	}

	//  Stage  2, butterflys and rotations
	{
		_LONG  x0, x1, x2;
		_INT   i0;

		i0 = 0;
		do
		{
			x0 = x[i0];
			x2 = x0;
			x1 = x[i0 + 16];
			x0 += x1;
			x2 -= x1;
			x[i0] = x0;
			x0 = _2C32[i0];
			x1 = x0;
			x0 >>= 8;
			x1 &= 0xFF;
			x0 = ((x2 * x0) >> 8);
			x1 = ((x2 * x1) >> 16);
			x0 += x1;
			x[i0 + 16] = x0;
			i0++;
		}
		while (i0 < 16);
	}

	//  Stage  3, 2 x 16-point FDCT
	FDCT16(x);
	FDCT16(x + 16);

	//  Stage  4, additional summarization
	{
		_LONG  x0, x1;
		_INT   x2;
		p_LONG  pX;

		pX = x + 16;
		x0 = *pX;
		x2 = 15;
		do
		{
			x1 = *(pX + 1);
			x0 += x1;
			*pX = x0;
			x0 = x1;
			pX++;
			x2--;
		}
		while (x2 > 0);
	}  //  End of Stage 3;

	//  Stage  5, second reordering
	{
		_LONG  x0, x1;
		_INT   i0, i1;
		for (i0 = 1; i0 < 6; i0 += 2)
		{
			i1 = i0;
			x1 = x[i1];
			do
			{
				i1 <<= 1;
				if (i1 > 31)
				{
					i1 -= 31;
				}
				x0 = x[i1];
				x[i1] = x1;
				x1 = x0;
			}
			while (i1 != i0);
		}

		for (i0 = 30; i0 > 25; i0 -= 2)
		{
			i1 = i0;
			x1 = x[i1];
			do
			{
				i1 <<= 1;
				if (i1 > 31)
				{
					i1 -= 31;
				}
				x0 = x[i1];
				x[i1] = x1;
				x1 = x0;
			}
			while (i1 != i0);
		}
	} //  End of Stage 4;

} // End of FDCT32 (p_LONG  x);





/************************************************************************/
/*   32 - Point  Inverse  Discrete  Cosine  Transform                   */
/************************************************************************/

_VOID   IDCT32(p_LONG  x)
{
	//  Stage  1, second reordering
	{
		_LONG  x0, x1;
		_INT   i0, i1, i2;
		for (i2 = 1; i2 < 6; i2 += 2)
		{
			i1 = i2;
			x0 = x[i1];
			do
			{
				i0 = i1;
				i1 <<= 1;
				if (i1 > 31)
				{
					i1 -= 31;
				}
				x1 = x[i1];
				x[i0] = x1;
			}
			while (i1 != i2);
			x[i0] = x0;
		}

		for (i2 = 30; i2 > 25; i2 -= 2)
		{
			i1 = i2;
			x0 = x[i1];
			do
			{
				i0 = i1;
				i1 <<= 1;
				if (i1 > 31)
				{
					i1 -= 31;
				}
				x1 = x[i1];
				x[i0] = x1;
			}
			while (i1 != i2);
			x[i0] = x0;
		}
	} //  End of Stage 4;

	//  Stage  2, Additional summarisation
	{
		_LONG  x0, x1;
		_INT   x2;
		p_LONG  pX;

		pX = x + 31;
		x0 = *pX;
		x2 = 15;
		do
		{
			x1 = *(pX - 1);
			x0 += x1;
			*pX = x0;
			x0 = x1;
			pX--;
			x2--;
		}
		while (x2 > 0);
	}  //  End of Stage 3;

	//  Stage  3, 2 x 16 point IDCT
	IDCT16(x);
	IDCT16(x + 16);

	//  Stage  4, Rotation and Butterfly
	{
		_LONG  x0, x1, x2;
		_INT   i0;
		i0 = 0;
		do
		{
			x2 = x[i0 + 16];
			x0 = _2C32[i0];
			x1 = x0;
			x0 >>= 8;
			x1 &= 0xFF;
			x0 = ((x2 * x0) >> 8);
			x1 = ((x2 * x1) >> 16);
			x0 += x1;    //  x0 = x2 * _2C01_16
			x1 = x[i0];
			x2 = x1;
			x1 += x0;
			x2 -= x0;
			x[i0] = x1;
			x[i0 + 16] = x2;
			i0++;
		}
		while (i0 < 16);
	}

	//  Stage  5, reordering
	{
		_LONG  x0, x1;
		_INT   i0, i1;
		i0 = 16;
		i1 = 31;
		do
		{
			x0 = x[i0];
			x1 = x[i1];
			x[i0] = x1;
			x[i1] = x0;
			i0++;
			i1--;
		}
		while (i0 < i1);
	}
} // End of IDCT31 (p_LONG  x);
