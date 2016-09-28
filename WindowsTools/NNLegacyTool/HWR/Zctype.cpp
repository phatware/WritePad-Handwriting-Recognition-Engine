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

#include "bastypes.h"
#ifndef HWR_SYSTEM_NO_LIBC
#include <ctype.h>
#endif
#include "zctype.h"

/*******************************************************************/
#define _2UC(n) (unsigned int)((n)&0x00ff)
#define _CHECK_RANGE(n) ((n) >= 0 && (n) <= 0xff)
#define _SEEK_RANGE  0x100
#define _ZTERM  '\0'

#define True 1
#define False 0

#ifndef _UPPER
#define _UPPER      0x1     /* upper case letter */
#define _LOWER      0x2     /* lower case letter */
#define _DIGIT      0x4     /* digit[0-9] */
#define _SPACE      0x8     /* tab, carriage return, newline, */
#endif // _UPPER
/* vertical tab or form feed */
#define _PUNCT      0x10    /* punctuation character */
#define _CONTROL    0x20    /* control character */
#define _BLANK      0x40    /* space char */
#define _HEX        0x80    /* hexadecimal digit */


ROM_DATA_EXTERNAL unsigned char  _xctype [];
ROM_DATA_EXTERNAL unsigned char  _xupper [];
ROM_DATA_EXTERNAL unsigned char  _xlower [];

/*******************************************************************/
int
#ifdef _FLAT32
IsUpper
#else
isupper
#endif /* _FLAT32 */
(int n)
{
	register unsigned int m;
	m = _2UC(n);
	if _CHECK_RANGE(m)
		if ((_xctype[m] & _UPPER) != 0)
		{
			return True;
		}

	return False;
}

/*******************************************************************/
int
#ifdef _FLAT32
IsLower
#else
islower
#endif /* _FLAT32 */
(int n)
{
	register unsigned int m;
	m = _2UC(n);
	if _CHECK_RANGE(m)
		if ((_xctype[m] & _LOWER) != 0)
		{
			return True;
		}

	return False;
}

/*******************************************************************/
int
#ifdef _FLAT32
IsPunct
#else
ispunct
#endif /* _FLAT32 */
(int n)
{
	register unsigned int m;
	m = _2UC(n);
	if _CHECK_RANGE(m)
		if ((_xctype[m] & _PUNCT) != 0)
		{
			return True;
		}

	return False;
}

/*******************************************************************/
int
#ifdef _FLAT32
IsAlnum
#else
isalnum
#endif /* _FLAT32 */
(int n)
{
#ifdef _FLAT32
	return (int) (IsAlpha(n) || IsDigit(n));
#else
	return (int)(isalpha(n) || isdigit(n));
#endif /* _FLAT32 */
}

/*******************************************************************/
int
#ifdef _FLAT32
IsAlpha
#else
isalpha
#endif /* _FLAT32 */
(int n)
{
#ifdef _FLAT32
	if (IsUpper(n) || IsLower(n))
	{
		return True;
	}

#else
	if (isupper(n) || islower(n))
	{
		return True;
	}
#endif /* _FLAT32 */
	return False;
}

/*******************************************************************/
int
#ifdef _FLAT32
ToUpper
#else
toupper
#endif /* _FLAT32 */
(int n)
{
	register unsigned int m;
	m = _2UC(n);
#ifdef _FLAT32

	if (IsAlpha(n))

#else
	if (isalpha(n))
#endif /* _FLAT32 */
#if HALF_TABLE
		if (m > 0x7f)
		{
			return (int) _xupper[m & 0x7f];
		}
		else
			if (m >= 'a' && m <= 'z')
			{
				return m - 0x20;
			}
#else
		return (int)_xupper[m];
#endif /* HALF_TABLE */

	return n;
}

/*******************************************************************/
int
#ifdef _FLAT32
ToLower
#else
tolower
#endif /* _FLAT32 */
(int n)
{
	register unsigned int m;

	m = _2UC(n);
#ifdef _FLAT32
	if (IsAlpha(n))
#else
	if (isalpha(n))
#endif /* _FLAT32 */
#if HALF_TABLE
		if (m > 0x7f)
		{
			return (int) _xlower[m & 0x7f];
		}
		else
			if (m >= 'A' && m <= 'Z')
			{
				return m + 0x20;
			}
#else
		return (int)_xlower[m];
#endif
	return n;
}

/*******************************************************************/
char *
#ifdef _FLAT32
StrLwr
#else
strlwr
#endif /* _FLAT32 */
(char * str)
{
	char *  buf;

	buf = str;
	if (str)
		while (*str != _ZTERM)
			*str++ =
#ifdef _FLAT32
			    (char)ToLower((int) *str);
#else
			    (char)tolower((int)*str);
#endif /* _FLAT32 */

	return buf;
}

/*******************************************************************/
char *
#ifdef _FLAT32
StrUpr
#else
strupr
#endif /* _FLAT32 */
(char * str)
{
	char *  buf;

	buf = str;
	if (str)
		while (*str != _ZTERM)
			*str++ =
#ifdef _FLAT32
			    (char)ToUpper((int) *str);
#else
			    (char)toupper((int)*str);
#endif /* _FLAT32 */

	return buf;
}

int IsDigit(int n)
{
#if defined(PEGASUS) || defined(HWR_SYSTEM_NO_LIBC)
	return (int)(n >= '0' && n <= '9') ? 1 : 0;
#else
	return (int) isdigit(n);
#endif
}
