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

#include "Fix32.h"

typedef unsigned long UnL;

#if kFractBits < 1 || kFractBits > 31
#error
#endif

#define kIntBits (32 - kFractBits)
Fixed32
FixMul32(Fixed32 a, Fixed32 b)
{
	long    sign = (a ^ b);
	UnL     na;
	UnL     nb;
	UnL     fa;
	UnL     fb;
	UnL     fab;

	if (a < 0)
	{
		a = -a;
	}
	if (b < 0)
	{
		b = -b;
	}

	na = (UnL) a >> 16;
	fa = (UnL) a - (na << 16);
	nb = (UnL) b >> 16;
	fb = (UnL) b - (nb << 16);

	fab = fa * fb;                          /* 00-32 */
	fa = nb * fa;                          /* 16-48 */
	fb = na * fb;                          /* 16-48 */
	na = na * nb;                          /* 32-64 */

	fa = fa + fb;                          /* 16-48 */
	fb = fab + (fa << 16);                 /* 00-32 */
	na += (((fab & (fa << 16)) | ((fab | (fa << 16)) & (~fb))) >> 31);  /* add carry */
	na = na + (fa >> 16);                  /* 32-64 */

	fa = fb + (0x80000000 >> kIntBits);     /* Round */
	na += ((fb & (~fa)) >> 31);             /* add carry */

	nb = na >> (32 - kIntBits - 1);
	if (nb != 0)
	{
		goto overflow;
	}
	nb = na << kIntBits;

	nb = nb | (fa >> (32 - kIntBits));

	// if(sign < 0) nb = -nb;
	return (sign < 0) ? -((Fixed32) nb) : ((Fixed32) nb);

overflow:
	nb = 0x80000000;
	if (sign >= 0)
	{
		nb--;
	}
	return (Fixed32) nb;

}

