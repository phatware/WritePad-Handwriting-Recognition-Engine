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

#include "hwr_sys.h"
#include "hwr_file.h"
#include "ams_mg.h"

#include "xrwdict.h"
#include "elk.h"
#define _TOPINE_INTERLAYER
#include "topine.h"
#undef _TOPINE_INTERLAYER

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

_HD xD_Dopen(_HFILE hfp)
{
	return 0;
}

_SHORT xD_Dsave(_HFILE hfp, _HD hd)
{
	return 1;
}

_VOID xD_Dfree(_HD hd)
{}

_SHORT xD_AddWord(p_UCHAR s, _SHORT inf, _HD hd)
{
	if (add_word(s, inf, (vocptr_type _PTR)hd))
	{
		goto err;
	}

	return 1;
err:
	return 0;
}

_SHORT xD_DelWord(p_UCHAR s, _HD hd)
{
	return 1;
}

_SHORT xD_GetHDB(_HDB _PTR phdb, p_UCHAR s, _SHORT n, _HD hd)
{
	return 1;
}

_VOID xD_InitGNT(_HDB hdb, _HD hd)
{}

_SHORT xD_GetNextTail(p_UCHAR s, p_SHORT inf, _HD hd)
{
	_UCHAR stat;

	if (tst_word(s, &stat, (vocptr_type _PTR)hd))
	{
		goto err;
	}

	*inf = (_SHORT) stat;

	return 1;
err:
	return 0;
}

_VOID xD_DestroyHDB(_HDB hdb, _HD hd)
{}

_HFILE xHWRFileOpen(_STR zPathName, _WORD wRdWrAccess, _WORD wOpenMode)
{
	return HWRFileOpen(zPathName, wRdWrAccess, wOpenMode);
}

_BOOL  xHWRFileClose(_HFILE hFile)
{
	return HWRFileClose(hFile);
}

#ifdef __cplusplus
}
#endif //__cplusplus


