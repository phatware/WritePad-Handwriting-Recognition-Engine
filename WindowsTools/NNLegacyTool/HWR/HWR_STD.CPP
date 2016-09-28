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
#include "hwr_sys.h"

#if HWR_SYSTEM != HWR_EPOC32
#include <stdlib.h>
#include <stdio.h>
#endif

/**************************************************************************
*                                                                         *
*    HWRAbs.                                                              *
*                                                                         *
**************************************************************************/

_INT  HWRAbs(_INT iArg)
{
	return (iArg > 0 ? iArg : -iArg);
}

_LONG  HWRLAbs(_LONG lArg)
{
	return (lArg > 0 ? lArg : -lArg);
}

/**************************************************************************
*                                                                         *
*    HWRItoa.                                                             *
*                                                                         *
**************************************************************************/

#if !defined(PEGASUS) && HWR_SYSTEM != HWR_EPOC32
_STR    HWRItoa(_INT iNumber, _STR pcString, _INT iRadix)
{

	p_CHAR pcRetcode;
	char *ptr;

	switch (iRadix)
	{
		default:
			ptr = "%0";
			break;
		case 10:
			ptr = "%d";
			break;
		case 16:
			ptr = "%x";
			break;
	}
#ifdef PEGASUS
	wvsprintf((LPTSTR)pcString,(LPCTSTR)ptr,iNumber);
#else
	sprintf(pcString, ptr, iNumber);
#endif
	pcRetcode = (p_CHAR) pcString;

	return(pcRetcode);

}
/**************************************************************************
*                                                                         *
*    HWRLtoa.                                                             *
*                                                                         *
**************************************************************************/

_STR    HWRLtoa(_LONG lNumber, _STR pcString, _INT iRadix)
{
	p_CHAR pcRetcode;
	char *ptr;

	switch (iRadix)
	{
		default:
			ptr = "%l0";
			break;
		case 10:
			ptr = "%ld";
			break;
		case 16:
			ptr = "%lx";
			break;
	}
	sprintf(pcString, ptr, lNumber);
	pcRetcode = (p_CHAR) pcString;

	return(pcRetcode);
}

#endif

#ifdef hasQD
/**************************************************************************
*                                                                         *
*    HWRRand.                                                             *
*                                                                         *
**************************************************************************/

_INT   HWRRand(_VOID)

{
	return rand();
}
#endif

#if HWR_SYSTEM != HWR_EPOC32
/**************************************************************************
*                                                                         *
*    HWRAtoi.                                                             *
*                                                                         *
**************************************************************************/

_INT      HWRAtoi(_STR pcString)
{
	return((_INT) atol(pcString));
}

/**************************************************************************
*                                                                         *
*    HWRAtol.                                                             *
*                                                                         *
**************************************************************************/

_LONG     HWRAtol(_STR pcString)
{
	return atol(pcString);
}
#endif
