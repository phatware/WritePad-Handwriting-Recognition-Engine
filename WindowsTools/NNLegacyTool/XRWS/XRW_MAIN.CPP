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
#include "ams_mg.h"                           /* Most global definitions     */
#include "xrword.h"                           /* Definition for use as engine*/
#include "xrword_p.h"
#include "postcalc.h"

#include "zctype.h"

#if PG_DEBUG
_INT xr_exp(p_xrdata_type xrdata, p_RWG_type rwg, p_rec_w_type prw, rc_type  _PTR rc);
#endif

_INT  xrlws(p_xrdata_type xrdata, p_RWG_type rwg, rc_type  _PTR rc);
_INT  JoinRWGs(p_RWG_type rwg1, p_RWG_type rwg2, p_RWG_type rwg);
_INT  xrlv(p_xrdata_type xrdata, p_RWG_type rwg, rc_type  _PTR rc);
_INT  xrsig(p_xrdata_type xrdata, p_RWG_type rwg, rc_type  _PTR rc);

#define XRWALG_XRLWSO 777 // Temp until removed

#define JRWG_DIFF 6       // RWG percent diff for joining graphs


/* ************************************************************************* */
/*      Combined caller of upper level routines + punctuation check          */
/* ************************************************************************* */
_INT  xrw_algs(p_xrdata_type xrdata, rec_w_type(_PTR rec_words)[NUM_RW], p_RWG_type rwg, rc_type  _PTR rc)
{
	_INT        ret_code;
	_INT        alg;
	_SHORT      m_xwm = rc->xrw_mode;
	_SHORT      m_xcm = rc->corr_mode;
	_SHORT      m_xwl = rc->xrw_max_wlen;
	_SHORT      m_xcp = rc->caps_mode;

	(*rec_words)[0].weight = 0;

	alg = rc->algorithm;

	if (alg == XRWALG_XRSL)
	{
		rc->xrw_max_wlen = 1;
		rc->xrw_mode = XRWM_CS;
	}

	if (rc->cmp_word[0])
	{
		alg = XRWALG_CMP;
	}

	switch (alg)
	{
		case XRWALG_XRSL:     //  {ret_code = xrsig(xrdata, rwg, rc); break;}
		case XRWALG_XRWS:     //  {ret_code = xrws(xrdata, rwg, rc); break;}
		case XRWALG_XRLW:     //  {ret_code = xrlw(xrdata, rwg, rc); break;}
		//    case XRWALG_XRLWS:      {ret_code = xrlws(xrdata, rwg, rc); break;}

		case XRWALG_XR_SPL:
		{
			ret_code = xrlv(xrdata, rwg, rc);
			break;
		}
		//    case XRWALG_XR_SPL:
		//    case XRWALG_XRLWSO:     {ret_code = xrlwso(xrdata, rwg, rc); break;}

		case XRWALG_CMP:
		{
			ret_code = GetCMPAliases(xrdata, rwg, rc->cmp_word, rc);
			break;
		}

#if PG_DEBUG
		case XRWALG_EXP:
		{
			ret_code = xr_exp(xrdata, rwg, &((*rec_words)[0]), rc);
			break;
		}
#endif

		default:
			goto err;
	}

	rc->xrw_max_wlen = m_xwl;
	rc->xrw_mode = m_xwm;
	rc->corr_mode = m_xcm;
	rc->caps_mode = m_xcp;

	return ret_code;
err:

	rc->xrw_max_wlen = m_xwl;
	rc->xrw_mode = m_xwm;
	rc->corr_mode = m_xcm;
	rc->caps_mode = m_xcp;

	return 1;
}

/* ************************************************************************* */
/* *    Test if cur xr is link type xr                                     * */
/* ************************************************************************* */
_INT is_xr_sp(_UCHAR xr)
{
	_UCHAR list [] = { X_FF, X_ZZZ, X_ZZ, X_Z, 0 };

	if (HWRStrChr((_STR) list, (_INT) xr))
	{
		return 1;
	}

	return 0;
}

/* ************************************************************************* */
/* *    Test if cur xr is dot-slash-... xr                                 * */
/* ************************************************************************* */
_INT is_xr_junk(_UCHAR xr)
{
	_UCHAR list [] = { X_FF, X_ZZZ, X_ZZ, X_Z, X_ZN,
	                   X_ST, X_DF, X_XT, X_XT_ST, X_AL,
	                   X_BL, X_BR, X_AR, X_TS, X_TZ,
	                   X_VS, X_SS, /*X_AN_UR, */ X_AN_UL, X_BSS,
#if defined (FOR_GERMAN) || defined (FOR_FRENCH) || defined (FOR_SWED) || defined (FOR_INTERNATIONAL)
	                   X_UMLAUT,
#endif /*FOR_GERMAN...*/
	                   0
	                 };

	if (HWRStrChr((_STR) list, (_INT) xr))
	{
		return 1;
	}

	return 0;
}

/* ************************************************************************* */
/* *    Test if cur xr is dot-slash-... xr                                 * */
/* ************************************************************************* */
_INT is_xr_dot(_UCHAR xr)
{
	_UCHAR list [] = { X_ST, X_XT, X_XT_ST,
#if defined (FOR_GERMAN) || defined (FOR_FRENCH) || defined (FOR_SWED) || defined (FOR_INTERNATIONAL)
	                   X_UMLAUT,
#endif /*FOR_GERMAN...*/
	                   0
	                 };

	if (HWRStrChr((_STR) list, (_INT) xr))
	{
		return 1;
	}

	return 0;
}
