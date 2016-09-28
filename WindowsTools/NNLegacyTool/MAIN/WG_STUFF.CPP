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
#include "hwr_sys.h"
#include "zctype.h"

#include "ams_mg.h"

#include "lowlevel.h"
#include "def.h"
#include "calcmacr.h"
#include "low_dbg.h"

#include "xrword.h"
#include "xrlw.h"

#include "postcalc.h"
#include "pdf_file.h"

#include "div_let.h"

#define _REQ_WIN
#include "wg_stuff.h"
#include "rec-main.h"
#include "recutil.h"

#define _UNUSE(v) ((void)(v))

#if PG_DEBUG
#include "pg_debug.h"

#define DBGTEST
#define printfw printw

#ifdef _FLAT32
#define FPTR(s,o) ((DWORD)(o))
#else /* _FLAT32 */
#define FPTR(s,o) (((((DWORD)(s))&0x0000ffff)<<16)+ \
				   (((DWORD)(o))&0x0000ffff))
#endif /* _FLAT32 */

typedef struct _LOWINK_TYPE
{
	_USHORT        LOIsSpecl;
	p_FSPECL       LOIpSpecl;
	p_SHORT        LOIxp;
	p_SHORT        LOIyp;
	_SHORT         LOInp;
	p_ULONG        LOIlRastr;
	p_ULONG        LOIhRastr;
	_SHORT         LOInRastr;
} LOWINK_TYPE;
typedef LOWINK_TYPE _PTR p_LOWINK;

_CHAR  buffer[128];
_SHORT  bwn;
_SHORT  buttons;
_SHORT  mouse_visible;

#ifdef _PENWIN
rec_w_type PPDwords[NUM_RW];
Osokin_output OData[NUM_RW];
#endif /* _PENWIN */

HWND hWndDbg = NULL;  /* debugger lab's main window handle */
HWND hBatchWnd = NULL;  /* debugger lab's dialog box window handle for special BATCH mode*/
_INT mFreeze = FALSE;

extern _UCHAR fl_cut;
extern rc_type rc;
extern rc_type rcm;
extern xrdata_type xrdata [];

rc_type xrc;
BOOL xrc_keep = FALSE;

/* useable macros */
#define ISDBG       (IsWindow(hWndDbg))
#define NODBG       {mFreeze=FALSE;hWndDbg=NULL;}
#define DBGMSG(h, w,l) SendMessage ((HWND)(h),WM_HWDBG,(WPARAM)(w),(LPARAM)(l))
#define DBGREQ(i,l) DebugRequest((UINT)(i),(LPARAM)(l))

#define STRANGER1 10000
#define STRANGER2 30
#define CLK_TSK   18.2

_USHORT	 WinKey2DosKey(_USHORT nWinVk);
_VOID	 DrawAndWait(p_LOWINK pLowInk);
_INT     GETWINLINESTYLE(_INT style);
COLORREF GETWINCOLOR(_INT color);
_SHORT   MaTprint(p_MPRINT pMP);
_SHORT   _Bfunction(p_CHAR word);

_SHORT   LayOutFunction(p_LAYOUTWORD lParam);
_INT     PrintHeightPenalties(p_rec_info_type pri);
_INT     WordTrajectory(p_WORDTRAJECTORY lParam);

/* =============================================================== */
/* =============================================================== */
/* =============================================================== */
#ifndef _PENWIN
_INT RECWORD_access(p_VOID pObject, /* root object Handle */
					_INT nCommand,      /* command id */
					_UINT uiParam,      /* multifunction uint parameter */
					_ULONG ulParam);    /* multifunction ulong parameter */

_INT XRDATA_access(p_VOID pObject,  /* root object Handle */
				   _INT nCommand,      /* command id */
				   _UINT uiParam,      /* multifunction uint parameter */
				   _ULONG ulParam);    /* multifunction ulong parameter */

_INT TRACE_access(p_VOID pObject,   /* root object Handle */
				  _INT nCommand,      /* command id */
				  _UINT uiParam,      /* multifunction uint parameter */
				  _ULONG ulParam);    /* multifunction ulong parameter */

_INT RWGRAPH_access(p_VOID pObject, /* root object Handle */
					_INT nCommand,      /* command id */
					_UINT uiParam,      /* multifunction uint parameter */
					_ULONG ulParam);    /* multifunction ulong parameter */

_INT RWGDATA_access(p_VOID pObject, /* root object Handle */
					_INT nCommand,      /* command id */
					_UINT uiParam,      /* multifunction uint parameter */
					_ULONG ulParam);    /* multifunction ulong parameter */

_INT RWGPPD_access(p_VOID pObject,  /* root object Handle */
				   _INT nCommand,      /* command id */
				   _UINT uiParam,      /* multifunction uint parameter */
				   _ULONG ulParam);    /* multifunction ulong parameter */

_INT BASELINE_access(p_VOID pObject, /* root object Handle */
					 _INT nCommand,      /* command id */
					 _UINT uiParam,      /* multifunction uint parameter */
					 _ULONG ulParam);    /* multifunction ulong parameter */

_INT RCONTEXT_access(p_VOID pObject, /* root object Handle */
					 _INT nCommand,      /* command id */
					 _UINT uiParam,      /* multifunction uint parameter */
					 _ULONG ulParam);    /* multifunction ulong parameter */

//??SD 09/26 add
_INT CORRECTORDATA_access(p_VOID pObject, /* root object Handle */
						  _INT nCommand,      /* command id */
						  _UINT uiParam,      /* multifunction uint parameter */
						  _ULONG ulParam);   /* multifunction ulong parameter */
//??SD 09/26 end
#endif /* _PENWIN */

/* =============================================================== */
/* =============================================================== */
/* =============================================================== */
static _INT  ReadMatrix(p_READMATRIX pData);
static _VOID GetMatrixData(p_rec_info_type pri, p_MATRIX_INPUT minp, p_READMATRIX pData);
#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */
/* =============================================================== */
/* =============================================================== */
/* =============================================================== */
_INT DLLEXPORT dbgSetDebugMode(_INT nMode, _ULONG lParam)
{
	switch (nMode)
	{

		case HWDBG_DEBUGLEVEL:           /* set debug level through "mpr" */
#ifndef  STRIP_PENLAB
#error Include header with STRIP_PENLAB !!!
#endif

#if  STRIP_PENLAB
			mpr = -6;
#else
			mpr = (_SHORT) LOWORD(lParam);
#endif  /*STRIP_PENLAB*/
			break;

		case HWDBG_LOOKLEVEL:           /* set view level through "look" */
			switch ((_SHORT) LOWORD(lParam))
			{
				case VL_NOPRINT:
					look = NOPRINT;
					break;
				case VL_WORDPRINT:
					look = WORDPRINT;
					break;
				case VL_VOCWPRINT:
					look = VOCWPRINT;
					break;
				case VL_MATRPRINT:
					look = MATRPRINT;
					break;
				case VL_LCUTPRINT:
					look = LCUTPRINT;
					break;
				case VL_CSTMPRINT:
					look = CSTMPRINT;
					break;
				case VL_XRLPRINT:
					look = XRLPRINT;
					break;
				case VL_XRCPRINT:
					look = XRCPRINT;
					break;
				default:
					break;
			}
			break;

		case HWDBG_WND:             /* set debug window */
			hWndDbg = (HWND) lParam;
			break;

		case HWBATCH_WND:           /* set debug window */
			hBatchWnd = (HWND) lParam;
			break;

		case HWDBG_XRDATA:
			/* not supported yet */
			break;

		case HWDBG_FREEZE:
			mFreeze = TRUE;
			break;

		case HWDBG_UNFREEZE:
			mFreeze = FALSE;
			break;

		case HWDBG_MATPRINT:
			if (!MaTprint((p_MPRINT) lParam))
			{
				printfw("\n Not enough memory to show matrix...");
			}
			break;

		case HWDBG_ZERO:
			NODBG;
			break;

		case HWDBG_GETWORDCUT:
			return (_INT) fl_cut & 0x00ff;

		case HWDBG_SETWORDCUT:
			fl_cut = LOBYTE(LOWORD(lParam));
			break;

		case HWDBG_GETSURELEVEL:
		{
#ifndef _PENWIN
			p_rec_info_type pri;
#endif
#ifdef _PENWIN
			return (_INT)rcm.sure_level;
#else
			pri = (p_rec_info_type) lParam;
			return ((p_rc_type) pri->prc)->sure_level;
#endif /* _PENWIN */

		}
		case HWDBG_SETSURELEVEL:
		{
#ifndef _PENWIN
			p_SETSURELEVEL  pParam;
			p_rec_info_type pri;
#endif
#ifdef _PENWIN
			rcm.sure_level = (_SHORT)(LOWORD(lParam));
#else
			pParam = (p_SETSURELEVEL) lParam;
			pri = (p_rec_info_type) pParam->pri;
			((p_rc_type) pri->prc)->sure_level = pParam->surelevel;
#endif
		}
		break;

		case HWDBG_LOADVOC:
		{
			p_VOCLOAD pV = (p_VOCLOAD) lParam;
			if (voc_load((p_UCHAR) pV->vocl_filename, (p_VOID _PTR)&(pV->vocl_ptr)) != 0)
			{
				return 0;
			}
			else
				if (pV->vocl_ptr != _NULL)
				{
					return 1;
				}
		}

		case HWDBG_FREEVOC:
		{
			/* use following call style SetDebugMode(HW.FREEVOC, (LPARAM)&ptr2voc)*/
			p_VOID _PTR  voc_ptr = (p_VOID _PTR)lParam;
			if (voc_ptr)
			{
				voc_unload(voc_ptr);
			}
			return 1;
		}

		case HWDBG_GETRC:
#ifdef _PENWIN
			if (lParam)
			{
				HWRMemCpy((p_VOID)lParam, (p_VOID)&rcm, sizeof(rc_type));
			}
#endif /* _PENWIN */
			break;

		case HWDBG_USENEWRC:
#ifdef _PENWIN
			if (!xrc_keep)
			{
				if (lParam)
				{
					HWRMemCpy((p_VOID)&xrc, (p_VOID)&rcm, sizeof(rc_type));
					xrc_keep = TRUE;
					HWRMemCpy((p_VOID)&rcm, (p_VOID)lParam, sizeof(rc_type));
				}
			}
#endif /* _PENWIN */
			break;

		case HWDBG_RESTORERC:
#ifdef _PENWIN
			if (xrc_keep)
			{
				HWRMemCpy((p_VOID)&rcm, (p_VOID)&xrc, sizeof(rc_type));
				xrc_keep = FALSE;
			}
#endif /* _PENWIN */
			break;

		case HWDBG_BFUNC:
		{
			_CHAR word[w_lim];
			if (!lParam)
			{
				*word = 0;
				printfw("\nEnter Word: ");
				scanw((p_CHAR) word);
			}
			else
			{
				HWRMemCpy((LPSTR) word, (LPSTR) lParam, w_lim);
			}
			if (*word)
			{
				_Bfunction((p_CHAR) word);
			}
			break;
		}

		case HWDBG_OFFLEARN:
		{
#ifdef _PENWIN
			xrwlp_type p;
			p.class_level = rcm.lrn_class_level;
			p.min_class_size = rcm.lrn_min_class;
			p.learn_suff = rcm.lrn_learn_suff;
			p.vocvar_reward = rcm.lrn_vocvar_rew;
			dti_lock(rcm.dtiptr);
			XrwLearn((rc_type _PTR)&rcm, (xrwlearn_type (_PTR)[300])lParam, &p);
			dti_unlock(rcm.dtiptr);
#endif /* _PENWIN */

			break;
		}
		case HWDBG_WORDLAYOUT:
			LayOutFunction((p_LAYOUTWORD) lParam);
			break;

		case HWDBG_WORDTRAJ:
			return WordTrajectory((p_WORDTRAJECTORY) lParam);

		case HWDBG_GETMATRIX:
		{
			p_READMATRIX  pData;
			pData = (p_READMATRIX) lParam;
			pData->nxr = ReadMatrix(pData);
		}
		break;

		case HWDBG_NEWFILE:
		{
			// Nothing to reset here!
		}
		break;
		default:
			break;
	}
	return nMode;
}

#ifdef __cplusplus
}            /* Assume C declarations for C++ */
#endif  /* __cplusplus */
/* =============================================================== */
_LONG DebugRequest(int nRequest, _LONG lParam)
{
	LRESULT l;
	l = 0L;
	if (hBatchWnd != NULL&&IsWindow(hBatchWnd))
	{
		l = DBGMSG(hBatchWnd, nRequest, lParam);
	}
	else
		if (hWndDbg != NULL&&IsWindow(hWndDbg))
		{
			l = DBGMSG(hWndDbg, nRequest, lParam);
		}
		else
		{
			NODBG;
		}
	return (_LONG) l;
}


/* =============================================================== */
/* init the dbggrxout clear the dbggrxout window
   and set drawing window parameters */
_VOID draw_init(_SHORT step,
				_SHORT x1, _SHORT y1, _SHORT x2, _SHORT y2,
				p_SHORT xp, p_SHORT yp, _SHORT np)
{
	DRAWINIT_TYPE di;
	_RECT rect;
	DBGTEST;
	if (!ISDBG)
	{
		NODBG;
		return;
	}
	else
	{

		size_cross(0, np - 1, xp, yp, (p_RECT) &rect);

		di.DIbox.left = rect.left;
		di.DIbox.right = rect.right;
		di.DIbox.top = rect.top;
		di.DIbox.bottom = rect.bottom;

		di.DImode = step;
		di.DIview.left = x1;
		di.DIview.top = y1;
		di.DIview.right = x2;
		di.DIview.bottom = y2;
		DBGREQ(DBG_DRAWINIT, (p_DRAWINIT) &di);
		OpenTextWindow(1);
	}
}

/* =============================================================== */

#ifdef __CPLUSPLUS
#error  XAXAXA
#endif  /* __cplusplus */

_INT  GetDTIVersionForPenlab(p_rec_info_type pri)
{
	rc_type _PTR     rc;
	p_dti_descr_type p_dti;
	_INT             ver;

	if (pri == _NULL || (rc = RIRCX(pri)) == _NULL)
	{
		return  -1;
	}

	dti_lock(rc->dtiptr);
	p_dti = (p_dti_descr_type) (RIRCX(pri)->dtiptr);
	if (p_dti == _NULL)
	{
		return -1;
	}

	ver = (HWRStrnCmp(p_dti->object_type, "DTI4", DTI_ID_LEN) == 0) ? 1 : 0;

	dti_unlock(RIRCX(pri)->dtiptr);

	return  ver;

}

/* =============================================================== */

static _INT ReadMatrix(p_READMATRIX pData)
{
	p_rec_info_type pri = pData->pri;
	p_PPD           pPPD = pData->ppd;
	p_LAB_XRDATA    pXR = pData->pxr_input;
	_INT            prototype_xr_index = pData->nxr, i, k;
	MATRIX_INPUT    minput;
	//_INT            result;

	dti_lock(RIRCX(pri)->dtiptr);
	//result = READ_MATRIX_ERROR;
	k = 0;
	while (pPPD->letter != 0)
	{
		i = 0;
		while (pPPD->xr[i] != 0 )
		{
			if (k == prototype_xr_index)
			{
				minput.xr = pXR[pPPD->alias[i]];
				minput.sym = pPPD->rletter;
				minput.nvariant = pPPD->num_var;
				minput.nxr = i;
				goto out;
			}
			i++;
			k++;
		}
		pPPD++;
	}
out:
	GetMatrixData(pri, &minput, pData);
	dti_unlock(RIRCX(pri)->dtiptr);
	return TRUE;
}

#if DTI_COMPRESSED
/* =============================================================== */
static void GetMatrixData(p_rec_info_type pri, p_MATRIX_INPUT minput, p_READMATRIX pData)
{
	_UCHAR            sym = (_UCHAR)OSToRec(minput->sym);
	p_dte_index_type      plt;
	p_dte_sym_header_type sfc;
	p_dte_var_header_type pvh;
	p_xrp_type            xrpv;
	_INT                  v, len, s;
	p_dti_descr_type p_dti = (p_dti_descr_type)(RIRCX(pri)->dtiptr);

	if (GetSymDescriptor(sym, minput->nvariant, &sfc, p_dti) < 0)
	{
		return;
	}
	if (GetSymIndexTable(sym, minput->nvariant, &plt, p_dti) < 0)
	{
		return;
	}
	if (GetVarHeader(sym, minput->nvariant, &pvh, p_dti) < 0)
	{
		return;
	}

	// xrpv - points to variant
	xrpv = pvh->xrs;
	xrpv += (minput->nxr-1);

	pData->version = (HWRStrnCmp(p_dti->object_type, "DTI2", DTI_ID_LEN) == 0) ? 1:0;
	// xrpv - points to xr
	pData->Pi = minput->xr.xdxr.xnp;
	pData->nVar = minput->nvariant;

#ifdef VERT_STD 
	pData->Cpenalty= xrpv->penl;
	pData->Cattrib = xrpv->attr;
	pData->version = 0;

	v =  minput->xr.xdxr.xnxr;
	pData->Cxr     = (v%2 == 0) ? xrpv->xtc[v/2] >> 4 : xrpv->xtc[v/2] & 0xF;
	v =  minput->xr.xdxr.xnh & 0xF;
	pData->Cheight = (v%2 == 0) ?  xrpv->xhc[v/2] >> 4 : xrpv->xhc[v/2] & 0xF;
	v =  minput->xr.xdxr.xnh >> 4;
	pData->Cshift  = (v%2 == 0) ?  xrpv->xsc[v/2] >> 4 : xrpv->xsc[v/2] & 0xF;

#else // DTV format - 
	pData->Cpenalty= xrpv->penl;
	pData->Cattrib = xrpv->attr;
	pData->version = 1;

	pData->Cxr     = plt->xt_palette[xrpv->xtc][minput->xr.xdxr.xnxr];
	pData->Cheight = plt->xh_palette[xrpv->xhc][minput->xr.xdxr.xnh];
	pData->Cshift  = plt->xs_palette[xrpv->xsc][minput->xr.xdxr.xns];
	pData->Corient = plt->xo_palette[xrpv->xoc][minput->xr.xdxr.xno];
	pData->Cdepth  = plt->xz_palette[xrpv->xzc][minput->xr.xdxr.xnd];

#endif

} /* end of GetMatrixData() */
#else
/* =============================================================== */
static void GetMatrixData(p_rec_info_type pri, p_MATRIX_INPUT minput, p_READMATRIX pData)
{
	_UCHAR					sym = (_UCHAR) OSToRec(minput->sym);
	p_let_table_type		plt;
	p_dte_sym_header_type	sfc;
	p_xrp_type				xrpv;
	_INT					v, len;
	p_dti_descr_type p_dti = (p_dti_descr_type) (RIRCX(pri)->dtiptr);

	if (sym < DTI_FIRSTSYM || sym >= DTI_FIRSTSYM + DTI_NUMSYMBOLS)
	{
		return;
	}

	plt = (p_let_table_type) (p_dti->p_dte);
	sfc = (p_dte_sym_header_type) ((p_UCHAR) plt + (_UINT) ((_ULONG) (*plt)[sym]));

	if ((p_VOID) sfc == (p_VOID) plt)
	{
		return;
	}
	if (sfc->num_vars == 0)
	{
		return;
	}

	xrpv = (p_xrp_type) ((p_UCHAR) sfc + sizeof(dte_sym_header_type));
	for (v = 0; v < minput->nvariant && v < DTI_MAXVARSPERLET; v++)
	{
		len = sfc->var_lens[v];
		xrpv += len;
	}

	// xrpv - points to variant
	xrpv += minput->nxr;

	pData->version = (HWRStrnCmp(p_dti->object_type, "DTI2", DTI_ID_LEN) == 0) ? 1 : 0;
	// xrpv - points to xr
	pData->Pi = minput->xr.xdxr.xnp;
	pData->nVar = minput->nvariant;

#ifdef VERT_STD 
	pData->Cpenalty= xrpv->penl;
	pData->Cattrib = xrpv->attr;
	pData->version = 0;

	v =  minput->xr.xdxr.xnxr;
	pData->Cxr     = (v%2 == 0) ? xrpv->xtc[v/2] >> 4 : xrpv->xtc[v/2] & 0xF;
	v =  minput->xr.xdxr.xnh & 0xF;
	pData->Cheight = (v%2 == 0) ?  xrpv->xhc[v/2] >> 4 : xrpv->xhc[v/2] & 0xF;
	v =  minput->xr.xdxr.xnh >> 4;
	pData->Cshift  = (v%2 == 0) ?  xrpv->xsc[v/2] >> 4 : xrpv->xsc[v/2] & 0xF;

#else // DTV format 
	pData->Cpenalty = xrpv->penl;
	pData->Cattrib = xrpv->attr;
	pData->version = 1;

	v = minput->xr.xdxr.xnxr;
	pData->Cxr = (v % 2 == 0) ? xrpv->xtc[v / 2] >> 4 : xrpv->xtc[v / 2] & 0xF;
	v = minput->xr.xdxr.xnh;
	pData->Cheight = (v % 2 == 0) ? xrpv->xhc[v / 2] >> 4 : xrpv->xhc[v / 2] & 0xF;
	v = minput->xr.xdxr.xns;
	pData->Cshift = (v % 2 == 0) ? xrpv->xsc[v / 2] >> 4 : xrpv->xsc[v / 2] & 0xF;
	v = minput->xr.xdxr.xno;
	pData->Corient = (v % 2 == 0) ? xrpv->xoc[v / 2] >> 4 : xrpv->xoc[v / 2] & 0xF;
	v = minput->xr.xdxr.xnd;
	pData->Cdepth = (v % 2 == 0) ? xrpv->xzc[v / 2] >> 4 : xrpv->xzc[v / 2] & 0xF;
#endif

} /* end of GetMatrixData() */
#endif
/* =============================================================== */
/* draws line in the dbggrxout */
_SHORT draw_line(_INT xbeg, _INT ybeg, _INT xend, _INT yend, _INT color, _INT style, _INT thickness)
{
	LINE_TYPE Line;
	DBGTEST;
	if (!ISDBG)
	{
		NODBG;
		return 0;
	}
	else
	{
		Line.Lcolor = GETWINCOLOR(color);
		Line.Lbeg.x = xbeg;
		Line.Lbeg.y = ybeg;
		Line.Lend.x = xend;
		Line.Lend.y = yend;
		Line.Lstyle = (_SHORT) GETWINLINESTYLE(style);
		Line.Lwidth = (_SHORT) thickness;
		return (_SHORT) DBGREQ(DBG_LINE, (p_LINE) &Line);
	}
}

/* =============================================================== */
/* draw in the dbggrxout sequence of input points in specified color */
_VOID draw_arc(_INT color, p_SHORT xx, p_SHORT yy, _INT ibeg, _INT iend)
{
	CURVE_TYPE Curve;
	DBGTEST;
	if (!ISDBG)
	{
		NODBG;
		return;
	}
	else
	{
		Curve.Ccolor = GETWINCOLOR(color);
		Curve.Cx = (p_SHORT) xx;
		Curve.Cy = (p_SHORT) yy;
		Curve.Cbeg = (_SHORT) ibeg;
		Curve.Cend = (_SHORT) iend;
		if (color < 0)
		{
			Curve.Cslow = TRUE;
		}
		else
		{
			Curve.Cslow = FALSE;
		}
		DBGREQ(DBG_CURVE, (p_CURVE) &Curve);
	}
}

/* =============================================================== */
/* draw full trace in the dbggrxout and also colors parts specified in specl */
_VOID draw_wait(p_VOID p_specl,
				p_SHORT xx, p_SHORT yy, _INT nn,
				p_ULONG lowrastr, p_ULONG hirastr,
				_INT nxrastr)
{
	p_SPECL specl;
	LOWINK_TYPE loi;
	_USHORT DataSeg = 0;

	DBGTEST;

	specl = (p_SPECL) p_specl;

#ifndef _FLAT32
	__asm
	{
		push ds
		pop  ax
		mov  DataSeg, ax
	}
#endif /* _FLAT32 */
	loi.LOIsSpecl = DataSeg;
	loi.LOIpSpecl = (p_FSPECL) FPTR(loi.LOIsSpecl, specl);
	loi.LOIxp = xx;
	loi.LOIyp = yy;
	loi.LOInp = (_SHORT) nn;
	loi.LOIlRastr = lowrastr;
	loi.LOIhRastr = hirastr;
	loi.LOInRastr = (_SHORT) nxrastr;
	if (!ISDBG)
	{
		NODBG;
	}
	else
	{
		/* bioskey(0); */
		if (lowrastr && hirastr && nxrastr)
		{
			dbgAddRaster(loi.LOIlRastr, loi.LOIhRastr, loi.LOInRastr);
		}
		DrawAndWait((p_LOWINK) &loi);

	}
}

/* =============================================================== */
/* output debug string to dbgout stream window (text stream mode) */
_INT printw(const p_CHAR fmt_str, ...)
{
	PRINTFW_TYPE PW;
	DBGTEST;
	if (!ISDBG)
	{
		NODBG;
		return 0;
	}
	else
	{
		PW.PWcolor = 0;
		PW.PWfont = 0;
		PW.PWpArgs = (p_CHAR _PTR)&fmt_str;
		return (_INT) DBGREQ(DBG_PRINTW, (p_PRINTFW) &PW);
	}
}

/* =============================================================== */
/* get string from dbgin stream, "enter" means end of input */
_SHORT scanw(p_CHAR inbuf)
{
	SCANW_TYPE Scanw;
	DBGTEST;
	if (!ISDBG)
	{
		NODBG;
		return 0;
	}
	else
	{
		*inbuf = '\0';
		Scanw.SCpBuf = inbuf;
		Scanw.SCnScanw = TRUE;
		DBGREQ(DBG_SCANW, (p_SCANW) &Scanw);
		if (*inbuf == '\0')
		{
			return 0;
		}
		return 1;
	}
}

/* =============================================================== */
/* opens dbgout window or clear dbgout if any exists */
_VOID OpenTextWindow(_INT lines)
{
	DBGTEST;
	lines = 0;
	if (!ISDBG)
	{
		NODBG;
		return;
	}
	else
	{
		DBGREQ(DBG_OPENTEXTWND, MAKELONG(lines, 0));
	}
}

/* =============================================================== */
/* closes dbgout window if any exists */
_VOID CloseTextWindow(_VOID)
{
	DBGTEST;
	if (!ISDBG)
	{
		NODBG;
		return;
	}
	else
	{
		DBGREQ(DBG_CLOSETEXTWND, 0);
	}
}

/* =============================================================== */
/* draw box in dbggrxout */
_VOID DrawBox(p_RECT pBox, _INT color, _INT style)
{
	DBGTEST;
	draw_line(pBox->left, pBox->top, pBox->right, pBox->top,
			  color, style, NORM_WIDTH);
	draw_line(pBox->right, pBox->top, pBox->right, pBox->bottom,
			  color, style, NORM_WIDTH);
	draw_line(pBox->right, pBox->bottom, pBox->left, pBox->bottom,
			  color, style, NORM_WIDTH);
	draw_line(pBox->left, pBox->bottom, pBox->left, pBox->top,
			  color, style, NORM_WIDTH);
}

/* =============================================================== */
_SHORT brkeyw(p_CHAR prompt)
{
	p_CHAR pBuf = (p_CHAR) &buffer[0];
	SCANW_TYPE Scanw;
	DBGTEST;
	if (!ISDBG)
	{
		NODBG;
		return 0;
	}
	else
	{
		printw(prompt);
		if (mpr <= -9)
		{
			return 0x3920;  /* SpaceBar */
		}
		Scanw.SCpBuf = (p_CHAR) pBuf;
		Scanw.SCnScanw = FALSE;
		DBGREQ(DBG_SCANW, (p_SCANW) &Scanw);
		return (_SHORT) WinKey2DosKey((_USHORT) *pBuf);
	}
}

/* =============================================================== */
int bioskey(int cmd)
{
	p_CHAR pBuf = (p_CHAR) &buffer[0];
	SCANW_TYPE Scanw;
	DBGTEST;
	if (!ISDBG)
	{
		NODBG;
		return 0;
	}
	else
	{
		if (cmd == 1)
		{
			return 1;
		}
		printw("\nPress a key...");
		Scanw.SCpBuf = (p_CHAR) pBuf;
		Scanw.SCnScanw = FALSE;
		DBGREQ(DBG_SCANW, (p_SCANW) &Scanw);
		return (_SHORT) WinKey2DosKey((_USHORT) *pBuf);
	}
}

/* =============================================================== */
/* soon will Set text showing mode */
_INT SetTalk(_BOOL bOn)
{
	DBGTEST;
	if (!ISDBG)
	{
		NODBG;
		return 0;
	}
	else
	{
		return (_INT) DBGREQ(DBG_SETTALKMODE, MAKELPARAM(bOn, 0));
	}
}

/* =============================================================== */
_USHORT WinKey2DosKey(_USHORT nWinVk)
{
	_USHORT n;
	n = (((MapVirtualKey(nWinVk, 0) & 0x00ff) << 8) | (nWinVk & 0x00ff));
	return n;
}

/* =============================================================== */
int sleep(int seconds)
{
	DBGTEST;
	if (!ISDBG)
	{
		NODBG;
		return 0;
	}
	else
	{
		DBGREQ(DBG_WAIT, (LPARAM) MAKELONG(seconds, 0));
	}
	return 1;
}

/* =============================================================== */
_VOID CheckPause(_VOID)
{
	DBGTEST;
	if (!ISDBG)
	{
		NODBG;
		return;
	}
	else
	{
#ifndef _FLAT32
		__asm {nop} /* place int 3 breakpoint here */
#endif /* _FLAT32 */
		return;
	}
}

/* =============================================================== */
long biostime(int cmd, long newtime)
{
	DBGTEST;
	_UNUSE(newtime);
	if (cmd == 1)
	{
		printw("IVB: request to set timer not yet supported...");
	}
	return (long) GetTickCount();
}

/* =============================================================== */
int delay(int ticks)
{
	long time, tick;
	DBGTEST;
	if (!ISDBG)
	{
		NODBG;
		return 0;
	}
	else
	{
		tick = ticks;
		time = biostime(0, 0L);
		while ((biostime(0, 0L) - time) < tick);
		return 1;
	}
}

/* =============================================================== */
_INT gprintf(_INT x, _INT y, _INT color, _INT spc, const p_CHAR fmt_str, ...)
{
	PRINTFW_TYPE PW;
	DBGTEST;
	_UNUSE(x);
	_UNUSE(y);
	_UNUSE(spc);
	if (!ISDBG)
	{
		NODBG;
		return 0;
	}
	else
	{
		PW.PWcolor = GETWINCOLOR(color);
		PW.PWfont = 0;
		PW.PWpArgs = (p_CHAR _PTR)&fmt_str;
		DBGREQ(DBG_PRINTW, (p_PRINTFW) &PW);
		return 1;
	}
}

/* =============================================================== */
void outtextxy(int x, int y, const char * textstring)
{
	DBGTEST;
	if (!ISDBG)
	{
		NODBG;
		return;
	}
	else
	{
		gprintf(x, y, 0, 0, (p_CHAR) textstring);
	}
}

/* =============================================================== */
_SHORT printansw(answ_type(_PTR answ)[NUM_RW])
{
	_SHORT   i, color;

	DBGTEST;
	printw("\n==================================");
	for (i = 0; i < NUM_RW && (*answ)[i].weight > 0; i++)
	{
		color = (*answ)[i].flags;
#if PG_COLOR
		if (color == 1)
		{
			color += 8;
		}
#endif
		printw("\n%s \t%d \t%3d",
			   (*answ)[i].word, (*answ)[i].stat, (*answ)[i].weight);
	}
	bioskey(0);
	return 0;
}

/* =============================================================== */
_SHORT put_xr(xrinp_type xr, _SHORT inc)
{
	XRDRAW_TYPE xrd;
	PRINTFW_TYPE PW;
	extern _UCHAR xr_colors [];
	DBGTEST;
	if (!ISDBG)
	{
		NODBG;
		return 0;
	}
	else
	{
#ifdef VERTIGO
		xrd.XDxr  = (_UCHAR)((_INT)xr.type+(190-32));
#else
		xrd.XDxr = (_UCHAR) ((_INT) xr.type);
#endif
		xrd.XDa = xr.attrib;
		xrd.XDp = xr.penalty;
		xrd.XDh = xr.height;
		xrd.XDinc = inc;
		PW.PWcolor = 0;
		PW.PWfont = 1;
		PW.PWpArgs = (p_CHAR _PTR)&xrd;
		return (_SHORT) DBGREQ(DBG_PRINTW, (p_PRINTFW) &PW);
	}
}

_SHORT WStub(_VOID)
{
	DBGTEST;
	printw("\nPlease wait: continuous operation or Your computer freeze...");
	return 0;
}

_SHORT hide_mouse(_VOID)
{
	DBGTEST;
	return 1;
}

_SHORT show_mouse(_VOID)
{
	DBGTEST;
	return 1;
}

_SHORT xrl_interface(_SHORT command, _SHORT param1, _SHORT param2,
					 p_VOID objptr, xrdata_type(_PTR xrd)[XRINP_SIZE])
{
	DBGTEST;
	_UNUSE(command);
	_UNUSE(param1);
	_UNUSE(param2);
	_UNUSE(objptr);
	_UNUSE(xrd);
	return 1;
}

/* =============================================================== */
_SHORT ViewELV(p_VOID dp, xrwlearn_type(_PTR xrwl)[XRWL_MAXW],
			   xrwlp_type _PTR params, p_VOID llv)
{
	LRNSTAT_TYPE ls;
	DBGTEST;
	if (!ISDBG)
	{
		NODBG;
		return 1;
	}
	else
	{
		ls.zdp = (p_VOID) dp;
		ls.zlv = (p_VOID) llv;
		ls.zld = (p_VOID) xrwl;
		ls.zlp = (p_VOID) params;
		return (_SHORT) DBGREQ(DBG_OFFLEARN, (p_LRNSTAT) &ls);
	}
}

/* =============================================================== */
_SHORT info_box
(p_CHAR line1, p_CHAR line2, p_CHAR line3, _SHORT ps)
{
	DBGTEST;
	printw(" ***  %s %s %s  *** ", line1, line2, line3);
	delay(ps);
	return 0;
}

void bar(int left, int top, int right, int bottom)
{
	DBGTEST;
	_UNUSE(left);
	_UNUSE(top);
	_UNUSE(right);
	_UNUSE(bottom);
}

int getmaxx(void)
{
	DBGTEST;
	return 1;
}

int getmaxy(void)
{
	DBGTEST;
	return 1;
}

void gotoxy(int x, int y)
{
	DBGTEST;
	_UNUSE(x);
	_UNUSE(y);
}

void set_timer(void)
{
	DBGTEST;
}

_VOID gp_ega(_SHORT x, _SHORT y, _SHORT color)
{
	DBGTEST;
	_UNUSE(x);
	_UNUSE(y);
	_UNUSE(color);
}

void putpixel(int x, int y, int color)
{
	DBGTEST;
	_UNUSE(x);
	_UNUSE(y);
	_UNUSE(color);
}

void setcolor(int color)
{
	DBGTEST;
	_UNUSE(color);
}

void setfillstyle(int pattern, int color)
{
	DBGTEST;
	_UNUSE(pattern);
	_UNUSE(color);
}

/* =============================================================== */
LRESULT RegData(_SHORT request, p_VOID pD)
{
	DBGTEST;

	if (!ISDBG && request != DBG_GETCMPWORD)
	{
		NODBG;
		return (LRESULT) NULL;
	}
	else
	{
		DBGREQ(request, (LPARAM) pD);
		return (LRESULT) pD;
	}
}

/* =============================================================== */
_VOID DbgFillGraphData(rc_type _PTR prc,
					   xrdata_type _PTR xrdata,
					   p_VOID p_rwg)
{
	RWS_type          _PTR pRWS;
	RWG_PPD_type      _PTR pPPDData;
	p_RWG_type        rwg = (p_RWG_type) p_rwg;

	dti_lock(prc->dtiptr);
	create_rwg_ppd(_NULL, prc, xrdata, rwg);
	dti_unlock(prc->dtiptr);
	pRWS = (RWS_type _PTR)rwg->rws_mem;
	pPPDData = (RWG_PPD_el_type(_PTR)[DTI_XR_SIZE + 1])rwg->ppd_mem;
	DbgFillGRData((p_VOID) rwg, (p_VOID) pRWS, (p_VOID) pPPDData, 0);
} /* end of DbgFillGraphData */

/* =============================================================== */
_VOID DbgFillGRData(p_VOID p_rwg, p_VOID pRWS, p_VOID pPPDData,_SHORT AfterPP)
{
	GRAPHDATA_TYPE      GR;
	p_GRAPH_TYPE        lpGraph, lpGrData;
	p_LAB_RWGNODE p_rws;
	p_LAB_PPNODE  p_ppd;

	GR.rwg_size = sizeof(GRAPH_TYPE);
	GR.rws_size = ((p_GRAPH_TYPE) p_rwg)->size;
	GR.p_rwg = NULL;
	GR.after_pp = AfterPP;
	RegData(DBG_GRAPHDATA, (p_VOID) (p_GRAPHDATA) &GR);
	if (GR.p_rwg != NULL)
	{
		//     //GR.p_rwg - points to the allocated structures
		lpGraph = (p_GRAPH_TYPE) GR.p_rwg;
		lpGrData = (p_GRAPH_TYPE) p_rwg;
		lpGraph->type = lpGrData->type;
		lpGraph->size = lpGrData->size;
		if (AfterPP)
		{
			p_rws = lpGraph->prwse;
			p_ppd = lpGraph->pppde;
		}
		else
		{
			p_rws = lpGraph->prwsb;
			p_ppd = lpGraph->pppdb;
			// in case if no second call to DbgFillGRData happens
			HWRMemCpy(lpGraph->prwse, pRWS, lpGrData->size*sizeof(RWS_type));
			HWRMemCpy(lpGraph->pppde, pPPDData, lpGrData->size*sizeof(RWG_PPD_type));
		}
		HWRMemCpy(p_rws, pRWS, lpGrData->size*sizeof(RWS_type));
		HWRMemCpy(p_ppd, pPPDData, lpGrData->size*sizeof(RWG_PPD_type));
		RegData(DBG_SORTGRAPH, (p_VOID) (p_GRAPHDATA) &GR);
	}
} /* end of DbgFillGRData */

/* =============================================================== */

_VOID DbgFillXRData(xrdata_type _PTR xrdata)
{
	GETXRDATA_TYPE XR;
	_INT i;

	if (!ISDBG)
	{
		NODBG;
		return;
	}
	XR.nxr = XRINP_SIZE;
	XR.lpxr = NULL;
	RegData(DBG_GETXRDATA, (p_VOID) (p_GETXRDATA) &XR);
	if (XR.lpxr == NULL)
	{
		return;
	}
	i = 0;
	while (i < XRINP_SIZE && i < xrdata->len)
	{
		XR.lpxr[i].xr = (*xrdata->xrd)[i].xr.type;
		XR.lpxr[i].a = (*xrdata->xrd)[i].xr.attrib;
		XR.lpxr[i].p = (*xrdata->xrd)[i].xr.penalty;
		XR.lpxr[i].h = (*xrdata->xrd)[i].xr.height;

		XR.lpxr[i].shift = (*xrdata->xrd)[i].xr.shift;
		XR.lpxr[i].orient = (*xrdata->xrd)[i].xr.orient;
		XR.lpxr[i].depth = (*xrdata->xrd)[i].xr.depth;

		XR.lpxr[i].begpoint = (*xrdata->xrd)[i].begpoint;
		XR.lpxr[i].endpoint = (*xrdata->xrd)[i].endpoint;

		i++;
	}
	if (i < XRINP_SIZE)
	{
		XR.lpxr[i].xr = 0;
		XR.lpxr[i].h = 0;
	}
} /* end of DbgFillXRData */

/* =============================================================== */

_INT WordTrajectory(p_WORDTRAJECTORY lParam)
{
#ifndef _PENWIN
	xrd_type        xr;
	p_LAB_XRDATA    pxr = lParam->pxr;
	p_LAB_RECWORD   pword = lParam->pword;
	p_LAB_RWGNODE   prwgnode = lParam->pgraph;
	p_WORDTRAJ      ptraj = lParam->ptraj;  // save output here
	int             i;


	// fill xr from PenLab data
	for (i = 0; i < LAB_XRINP_SIZE; i++)
	{
		xr[i].xr.type = pxr[i].xdxr.xnxr;
		xr[i].xr.attrib = pxr[i].xdxr.xna;
		xr[i].xr.penalty = pxr[i].xdxr.xnp;
		xr[i].xr.height = pxr[i].xdxr.xnh;

		xr[i].xr.shift = pxr[i].xdxr.xns;
		xr[i].xr.orient = pxr[i].xdxr.xno;
		xr[i].xr.depth = pxr[i].xdxr.xnd;

		xr[i].hotpoint = pxr[i].xdhotpt;
		xr[i].begpoint = pxr[i].xdbegpt;
		xr[i].endpoint = pxr[i].xdendpt;
		xr[i].box_left = pxr[i].xdleft;
		xr[i].box_up = pxr[i].xdtop;
		xr[i].box_right = pxr[i].xdright;
		xr[i].box_down = pxr[i].xdbottom;
	}
	return 0;
#else /* _PENWIN */
	xrdata_type _PTR  xr   = lParam->xr;
	rec_w_type _PTR   word = lParam->word;
	pOsokin_output  traj = lParam->traj;
	_SHORT          i, j, k;

	OData[0].pParts_of_letters = NULL;
	lParam->count = 0;
	if (connect_trajectory_and_answers(xr, word, &OData[0])==SUCCESS)
	{
		k = i = 0;
		while(i < w_lim && OData[0].num_parts_in_letter[i] != 0)
		{
			traj->num_parts_in_letter[i] = OData[0].num_parts_in_letter[i];
			k += OData[0].num_parts_in_letter[i];
			i++;
		}
		traj->num_parts_in_letter[i] = 0;
		HWRMemCpy((p_VOID)traj->pParts_of_letters, (p_VOID)OData[0].pParts_of_letters,
				  k*sizeof(Part_of_letter));
		lParam->count = k;
		return 1;
	}
	return 0;
#endif /* _PENWIN */
} /* end of WordTrajectory */

/* =============================================================== */
_VOID DbgFillHLData(rc_type _PTR prc,
					xrdata_type(_PTR xrdata)[XRINP_SIZE],
					p_VOID p_answr,
					rec_w_type(_PTR rec_words)[NUM_RW])
{
#ifdef _PENWIN
	HILODATA_TYPE HL;
	_INT n, m, i, j, k;
	ppd_type _PTR ppd_ptr;
	answ_type (_PTR answr)[NUM_RW];

	if (!ISDBG)
	{
		NODBG;
		return;
	}

	answr = (answ_type(_PTR)[NUM_RW])p_answr;

	HL.HLnwords = NUM_RW;
	HL.HLwords = NULL;
	for(HL.HLnppd = 1, n = 0; n < NUM_RW && \
			(*rec_words)[n].word[0]; n ++)
	{
		HL.HLnppd += (HWRStrLen((_STR)(*rec_words)[n].word)+1);
	}
	HL.HLppd = NULL;
	for (n = 0; n < NUM_RW; n++)
	{
		PPDwords[n].src_id = (*rec_words)[n].src_id;
		PPDwords[n].weight = (*rec_words)[n].weight;
		for (m = 0; m < w_lim; m++)
		{
			PPDwords[n].word[m]  = (*rec_words)[n].word[m];
			PPDwords[n].alias[m] = (*rec_words)[n].linp[m] << 4;
			PPDwords[n].alias[m] |= (*rec_words)[n].nvar[m] &0x0f;
		}
	}
	ppd_ptr = NULL;
#if USE_POSTPROC
	//  create_ppd(&rc, xrdata, &PPDwords, &ppd_ptr);
#endif
	/* calc there */
	for (n = 0; n < NUM_RW; n++)
	{
		OData[n].pParts_of_letters = NULL;
	}
	/* cycle for each word in PPDwords */
	for (m = 1, n = 0; n < NUM_RW && PPDwords[n].word[0]; n++)
	{
		if(connect_trajectory_and_answers(*xrdata,&(PPDwords[n]),&(OData[n]))!=SUCCESS)
		{
			printfw("\nDIV_LET: Not proceed! Word: %s",
					&(PPDwords[n].word));
		}
		else
		{
			/* calc space for OData[n].beg/end array for each word */
			/* cycle for each letter in n-word */
			for (i = 0; i < w_lim && PPDwords[n].word[i]; i++)
			{
				m += OData[n].num_parts_in_letter[i];
			}
		}
	}
	if (ppd_ptr != NULL)
	{
		HL.HLnintervals = m;
		HL.HLintervals = NULL;
		HL.HLnparts = HL.HLnppd;
		HL.HLparts = NULL;
	}
	else
	{
		HL.HLnppd = 0;
		HL.HLnparts = HL.HLnppd;
		HL.HLparts = NULL;
		HL.HLnintervals = 0;
		HL.HLintervals = NULL;
	}
	if (RegData(DBG_HILODATA, (p_VOID)(p_HILODATA)&HL) == (LRESULT)NULL) ;

	if (HL.HLwords != NULL)
	{
		for (n = 0; n < NUM_RW; n++)
		{
			HL.HLwords[n].stat    = (*answr)[n].stat;
			HL.HLwords[n].weight  = (*answr)[n].weight;
			HL.HLwords[n].percent = (*answr)[n].percent;
			HL.HLwords[n].flags   = (*answr)[n].flags;
			for (m = 0; m < w_lim; m++)
			{
				HL.HLwords[n].word[m]  = (*rec_words)[n].word[m];
				HL.HLwords[n].alias[m]  = (*rec_words)[n].linp[m] << 4;
				HL.HLwords[n].alias[m] |= (*rec_words)[n].nvar[m] & 0x0f;
			}
		}
	}
	if (HL.HLppd != NULL)
	{
		if (ppd_ptr != NULL)
		{
			for (n = 0; n < HL.HLnppd; n++)
			{
				HL.HLppd[n].letter = ppd_ptr[n].letter;
				HL.HLppd[n].percent = ppd_ptr[n].percent;
				HL.HLppd[n].num_var = ppd_ptr[n].num_var;
				for (m = 0; m <= LAB_XR_SIZE && m <= XR_SIZE; m++)
				{
					HL.HLppd[n].xr[m] = ppd_ptr[n].xr[m];
					HL.HLppd[n].attr[m] = ppd_ptr[n].attr[m];
					HL.HLppd[n].alias[m] = ppd_ptr[n].alias[m];
				}
			}
		}
	}
	if (HL.HLparts != NULL)
	{
		for(m = 0, n = 0; n < NUM_RW && PPDwords[n].word[0]; n++)
		{
			for (i = 0; i < w_lim && PPDwords[n].word[i]; i++)
			{
				HL.HLparts[i+m] = OData[n].num_parts_in_letter[i];
			}
			m += i;
			HL.HLparts[m] = 0;
			m++;
		}
		HL.HLparts[m] = 0;
	}
	if (HL.HLintervals != NULL)
	{
		/* cycle for each word in PPDwords */
		for (m = 0, n = 0; n < NUM_RW && PPDwords[n].word[0]; n++)
		{
			/* cycle for each letter in n-word */
			for (k = 0, i = 0; i < w_lim && PPDwords[n].word[i]; i++)
			{
				/* cycle with OData values */
				for (j = 0; j < OData[n].num_parts_in_letter[i]; j++)
				{
					if (OData[n].pParts_of_letters != NULL)
					{
						HL.HLintervals[j+m].ibeg = \
												   OData[n].pParts_of_letters[j+k].ibeg;
						HL.HLintervals[j+m].iend = \
												   OData[n].pParts_of_letters[j+k].iend;
					}
					else
					{
						HL.HLintervals[j+m].ibeg = 0;
						HL.HLintervals[j+m].iend = 0;
					}
				}
				k += OData[n].num_parts_in_letter[i];
				m += OData[n].num_parts_in_letter[i];
			}
		}
	}
	if (ppd_ptr != NULL)
	{
		PPD_dealloc(&ppd_ptr);
	}
	for (n = 0; n < NUM_RW; n++)
		if (OData[n].pParts_of_letters != NULL)
		{
			HWRMemoryFree(OData[n].pParts_of_letters);
		}
#endif /* _PENWIN */
}

/* =============================================================== */

_VOID DbgFillTraceData(PS_point_type _PTR pts, _SHORT np)
{
	TRACEDATA_TYPE Tr;
	_INT n;

	if (!ISDBG)
	{
		NODBG;
		return;
	}
	Tr.TDnp = np;
	Tr.TDpt = NULL;
	if (RegData(DBG_TRACEDATA, (p_VOID) (p_TRACEDATA) &Tr) == (LRESULT) NULL)
	{
		return;
	}
	if (Tr.TDpt != NULL)
	{
		for (n = 0; n < np; n++)
		{
			Tr.TDpt[n].x = pts[n].x;
			Tr.TDpt[n].y = pts[n].y;
		}
	}
}

/* =============================================================== */
_VOID DbgFillBorderData(p_USHORT pts, _SHORT np)
{
	TRACEDATA_TYPE Tr;
	_INT n;
	p_USHORT pData;

	if (!ISDBG)
	{
		NODBG;
		return;
	}
	Tr.TDnp = np;
	Tr.TDpt = NULL;
	if (RegData(DBG_LINEDATA, (p_VOID) (p_TRACEDATA) &Tr) == (LRESULT) NULL)
	{
		return;
	}
	if (Tr.TDpt != NULL)
	{
		pData = (p_USHORT) Tr.TDpt;
		for (n = 0; n < np; n++)
		{
			pData[n] = pts[n];
		}
	}
}

/* =============================================================== */
_VOID DbgFillAWData(p_VOID p_answer)
{
	ANSWERDATA_TYPE Result;
	_INT m, n;
	p_ANSWER pan;
	answ_type(_PTR answers)[NUM_RW];

	if (!ISDBG)
	{
		NODBG;
		return;
	}
	answers = (answ_type(_PTR)[NUM_RW])p_answer;

	Result.nanswers = NUM_RW;
	Result.answers = NULL;
	if (RegData(DBG_ANSWERSDATA, (p_VOID) (p_ANSWERDATA) &Result) == (LRESULT) NULL)
	{
		return;
	}
	if (Result.answers != NULL)
	{
		pan = (p_ANSWER) Result.answers;
		for (m = 0; m < NUM_RW; m++)
		{
			for (n = 0; n < w_lim && n < LAB_RW_SIZE; n++)
			{
				pan->anword[n] = (*answers)[m].word[n];
			}
			pan->anweight = (*answers)[m].weight;
			pan->anstat = (*answers)[m].stat;
			pan->anpercent = (*answers)[m].percent;
			pan->anflags = (*answers)[m].flags;
			pan->ansources = (*answers)[m].sources;
			pan->anchain_num = (*answers)[m].chain_num;
			pan->ansrc_id = (*answers)[m].src_id;
			pan++;
		}
	}
} /* end of DbgFillAWData */

/* =============================================================== */
_VOID DbgFillRWData(rec_w_type(_PTR rec_words)[NUM_RW])
{
	RECWORDSDATA_TYPE Result;
	_INT n, m;
	p_LAB_RECWORD prw;

	if (!ISDBG)
	{
		NODBG;
		return;
	}
	Result.nwords = NUM_RW;
	Result.words = NULL;
	if (RegData(DBG_RECWORDSDATA, (p_VOID) (p_RECWORDSDATA) &Result) == (LRESULT) NULL)
	{
		return;
	}
	if (Result.words != NULL)
	{
		prw = (p_LAB_RECWORD) Result.words;
		for (m = 0; m < NUM_RW; m++)
		{
			for (n = 0; n < w_lim && n < LAB_RW_SIZE; n++)
			{
				prw->rwword[n] = (*rec_words)[m].word[n];
				prw->rwlinp[n] = (*rec_words)[m].linp[n];
				prw->rwnvar[n] |= (*rec_words)[m].nvar[n];
			}
			prw->rwweight = (*rec_words)[m].weight;
			prw->rwsource = (*rec_words)[m].src_id;
			prw->rwdattrib = (*rec_words)[m].attr;
			if (prw->rwword[0] == 0)
			{
				break;
			}
			prw++;
		}
	}
}  /* end of DbgFillRWData */


/* =============================================================== */
_VOID DbgFillChunkData(p_VOID pChunkData)
{
	if (!ISDBG)
	{
		NODBG;
		return;
	}
	if (RegData(DBG_CHUNKDATA, pChunkData) == (LRESULT) NULL)
	{
		return;
	}
} /* end of DbgFillChunkData */

/* =============================================================== */
_VOID DbgLetterPos(p_VOID pLetterPos)
{
	if (!ISDBG)
	{
		NODBG;
		return;
	}
	if (RegData(DBG_LETTERPOS, pLetterPos) == (LRESULT) NULL)
	{
		return;
	}
} /* end of DbgFillChunkData */


/* =============================================================== */
_VOID DbgGetCMPWord(rec_w_type(_PTR rec_words)[NUM_RW])
{
#ifdef _PENWIN
	p_CHAR ptr;

	RegData(DBG_GETCMPWORD, (p_VOID)rec_words);
	(*rec_words)[0].weight = 100;

	if ((*rec_words)[0].word[0] == '#')  // Forget  cmp-closed words
	{
		(*rec_words)[0].word[0] = 0;
		(*rec_words)[0].weight  = 0;
	}

	if ((ptr = HWRStrChr((*rec_words)[0].word, '|')) != _NULL) // Take only first var from OR
	{
		*ptr = 0;
#else // !_PENWIN
	if (RegData(DBG_GETCMPWORD, (p_VOID) rec_words) == (LRESULT) NULL)
	{
		return;
	}
#endif // _PENWIN
	} /* end of DbgGetCMPWord */

	/* =============================================================== */
	/* converts bgi line styles to the windows line styles */
	_INT GETWINLINESTYLE(_INT style)
	{
		switch (style)
		{
			case DOTTED_LINE:
					return PS_DOT;
			case CENTER_LINE:
					return PS_DASHDOT;
			case DASHED_LINE:
					return PS_DASH;
			case USERBIT_LINE:
					return PS_SOLID;
			case SOLID_LINE:
				default:
						return PS_SOLID;
		}
	}

	/* =============================================================== */
#define BLUEBR  BLUEH|BLUEL
#define GREENBR GREENH|GREENL
#define REDBR   REDH|REDL
#define BLUEH   0x01
#define BLUEL   0x08
#define GREENH  0x02
#define GREENL  0x10
#define REDH    0x04
#define REDL    0x20
#define YH      0x02
#define YL      0x01

	/* convert bgi colors to the windows colors */
	COLORREF GETWINCOLOR(_INT color)
	{
		BYTE r, g, b;
		if (color < 0)
		{
			color = -color;
		}
		if (color > 7 && color < 16) /* CGA2EGA mapping */
		{
			color += 48;
		}
		r = ((color&REDH ? YH : 0) + (color&REDL ? YL : 0)) * 85;
		g = ((color&GREENH ? YH : 0) + (color&GREENL ? YL : 0)) * 85;
		b = ((color&BLUEH ? YH : 0) + (color&BLUEL ? YL : 0)) * 85;
		return RGB(r, g, b);
	}

	/* =============================================================== */
	_VOID DrawAndWait(p_LOWINK pLowInk)
	{
		p_USHORT pBuf = (p_USHORT) &buffer[0];
		SCANW_TYPE Scanw;
		p_FSPECL pSpecl;
		_SHORT color;
		_INT bDraw;

		/* place for draw init */

		color = COLOR;

		pSpecl = (p_FSPECL) pLowInk->LOIpSpecl;
		while (pSpecl != NULL)
		{
			bDraw = TRUE;
			switch (pSpecl->mark)
			{
				case BEG:
					case END:
							break;
				case MINW:
						if (mpr != 3)
						{
							color = COLORMIN;
						}
						else
						{
							bDraw = FALSE;
						}
					break;
				case MINN:
						if (mpr != 3)
						{
							color = COLORMINN;
						}
						else
						{
							bDraw = FALSE;
						}
					break;
				case MAXW:
						if (mpr != 3)
						{
							color = COLORMAX;
						}
						else
						{
							bDraw = FALSE;
						}
					break;
				case MAXN:
						if (mpr != 3)
						{
							color = COLORMAXN;
						}
						else
						{
							bDraw = FALSE;
						}
					break;
				case ANGLE:
						if (mpr != 3)
						{
							color = COLORAN;
						}
						else
						{
							bDraw = FALSE;
						}
					break;
				case CROSS:
					case STICK:
						case HATCH:
								if (mpr != 3)
								{
									color = COLORC;
								}
								else
								{
									bDraw = FALSE;
								}
					break;
				case STROKE:
						color = COLORT;
					break;
				case DOT:
						color = COLORP;
					break;
				case SHELF:
						color = COLORSH;
					break;
				default:
						bDraw = FALSE;
					break;
			}
			if (bDraw == TRUE)
			{
				draw_arc(color, pLowInk->LOIxp, pLowInk->LOIyp,
						 pSpecl->ibeg, pSpecl->iend);
			}
			if (pSpecl->next == NULL)
			{
				pSpecl = NULL;
			}
			else
			{
				pSpecl = (p_FSPECL) FPTR(pLowInk->LOIsSpecl, pSpecl->next);
			}
		}
		if (mpr != 2)     /*CHE*/
		{
			printw("\nPress mouse or key to continue ...");
			Scanw.SCpBuf = (p_CHAR) pBuf;
			Scanw.SCnScanw = FALSE;
			DBGREQ(DBG_SCANW, (p_SCANW) &Scanw);
		}
	}

	/* =============================================================== */
	_SHORT MaTprint(p_MPRINT pMP)
	{
		_SHORT i, use_v_inf;
		_SHORT m_cm, m_ba;
		xrcm_type _PTR xrcm;
		_SHORT          result;
#ifndef _PENWIN
		rec_info_type _PTR pri = (rec_info_type _PTR)pMP->pInfo;
		xrd_el_type _PTR xrdata = (p_xrd_el_type) (((p_xrdata_type) pri->pxrdata)->xrd);
#endif /* _PENWIN */

		if (pMP->MPnxr == 0 || pMP->MPxr == NULL)
		{
			return 0;
		}
		for (i = 0; i < pMP->MPnxr; i++)
		{
			xrdata[i].xr.type = pMP->MPxr[i].xdxr.xnxr;
			xrdata[i].xr.attrib = pMP->MPxr[i].xdxr.xna;
			xrdata[i].xr.penalty = pMP->MPxr[i].xdxr.xnp;
			xrdata[i].xr.height = pMP->MPxr[i].xdxr.xnh;

			xrdata[i].xr.shift = pMP->MPxr[i].xdxr.xns;
			xrdata[i].xr.orient = pMP->MPxr[i].xdxr.xno;
			xrdata[i].xr.depth = pMP->MPxr[i].xdxr.xnd;

			xrdata[i].hotpoint = pMP->MPxr[i].xdhotpt;
			xrdata[i].begpoint = pMP->MPxr[i].xdbegpt;
			xrdata[i].endpoint = pMP->MPxr[i].xdendpt;
			xrdata[i].box_left = 0;
			xrdata[i].box_up = 0;
			xrdata[i].box_right = 0;
			xrdata[i].box_down = 0;
		}
		m_cm = RIRCX(pri)->caps_mode;
		m_ba = RIRCX(pri)->bad_amnesty;
		RIRCX(pri)->caps_mode = pMP->CapsMode;
		RIRCX(pri)->bad_amnesty = pMP->BadAmnisty;
#ifndef _PENWIN
		dti_lock(RIRCX(pri)->dtiptr);
		if (xrmatr_alloc(RIRCX(pri), (p_xrdata_type) pri->pxrdata, &xrcm) != 0)
		{
			dti_unlock(RIRCX(pri)->dtiptr);
			return 0;
		}
#else /* _PENWIN */
		dti_lock(rcm.dtiptr);
		if (xrmatr_alloc(&rcm, &xrdata, &xrcm) != 0)
		{
			dti_unlock(rcm.dtiptr);
			return 0;
		}
#endif /* _PENWIN */

		if ((pMP->MPflags&MP_DIRECTION) != 0)
		{
			_strrev(pMP->MPtext);
		}
		change_direction(((pMP->MPflags&MP_DIRECTION) == 0) ? 0 : 1, xrcm);            /* Set everything to dir pass  */
		SetInitialLine(DTI_XR_SIZE / 2, xrcm);
		HWRStrCpy((_STR) xrcm->word, pMP->MPtext);
		use_v_inf = ((pMP->MPflags&MP_VIEWEXTRAINFO) == 0) ? 0 : XRMC_USELEARNINF;

		use_v_inf |= XRMC_DOTRACING;

		SetInitialLine(DTI_XR_SIZE / 3, xrcm);
		CountWord(xrcm->word, xrcm->caps_mode, use_v_inf, xrcm);

#ifndef _PENWIN
		if (!pMP->PrintMatrix)
		{
			result = xrcm->wwc_pos;
			goto out;
		}

#endif // !_PENWIN
		if ((pMP->MPflags & MP_VIEWCORRELATION) != 0)
		{
			look = MATRPRINT;
		}
		else
		{
			look = NOPRINT;
		}

		SetInitialLine(DTI_XR_SIZE / 3, xrcm);
		CountWord(xrcm->word, xrcm->caps_mode, use_v_inf, xrcm);
		FreeLayout(xrcm);
		look = NOPRINT;
		printfw("\n\n%s  Weight: %d, wc: %d, wwc: %d, wwc_pos: %d, X/L: %d",
				xrcm->word, (_INT) GetFinalWeight(xrcm), (_INT) xrcm->wc, (_INT) xrcm->wwc, (_INT) xrcm->wwc_pos,
				(_INT) (100.0*(float) (xrcm->xrinp_len) / ((float) HWRStrLen((_STR) xrcm->word))));
		out:
		xrmatr_dealloc(&xrcm);
		RIRCX(pri)->caps_mode = m_cm;
		RIRCX(pri)->bad_amnesty = m_ba;
#ifndef _PENWIN
		dti_unlock(RIRCX(pri)->dtiptr);
#else /* _PENWIN */
		dti_unlock(rcm.dtiptr);
#endif /* _PENWIN */
		return 1;
	}

	/* =============================================================== */
#ifndef _PENWIN
	_SHORT LayOutFunction(p_LAYOUTWORD lpLocData)
	{
		rec_info_type _PTR  lpData = lpLocData->pRCInfo;
		_UINT               i, save;
		_SHORT              wwc_pos;
		MPRINT_TYPE         Matrix;
		rc_type _PTR        pin;
		LAB_XRDATA_TYPE     savedxr;
#else // _PENWIN
	_SHORT LayOutFunction(p_LAYOUTWORD lpData)
	{
#endif //_PENWIN

		if (RISYGRAPH(lpData) == _NULL)
		{
			return 0;
		}

#ifndef _PENWIN
		if (lpLocData->SpMode)
		{
			Matrix.MPrange[0] = -1;
			Matrix.MPflags = MP_VIEWEXTRAINFO;
			Matrix.pInfo = lpData;
			pin = (rc_type _PTR)(lpData)->prc;
			Matrix.CapsMode = pin->caps_mode;
			Matrix.BadAmnisty = pin->bad_amnesty;
			// copy word
			i = 0;
			while (lpLocData->Word[i] != 0)
			{
				Matrix.MPtext[i] = lpLocData->Word[i];
				i++;
			}
			Matrix.MPtext[i] = 0;
			// copy xr data
			Matrix.MPnxr = 0;
			while (lpLocData->pxr[Matrix.MPnxr].xdxr.xnxr != 0)
			{
				Matrix.MPnxr++;
			}
			Matrix.MPxr = lpLocData->pxr;
			Matrix.PrintMatrix = FALSE;
			wwc_pos = MaTprint(&Matrix);
			if (wwc_pos != 0)
			{
				// cut xr string
				savedxr = Matrix.MPxr[wwc_pos + 1];
				save = Matrix.MPxr[wwc_pos + 2].xdxr.xnxr;
				Matrix.MPxr[wwc_pos + 1] = Matrix.MPxr[Matrix.MPnxr - 1];
				Matrix.MPxr[wwc_pos + 2].xdxr.xnxr = 0;
				i = RCBAccess((void FAR *)lpData, OBJ_XRDATA, CMD_SET,
							  (_UINT) wwc_pos + 3, (_ULONG) Matrix.MPxr);
			}
			else
			{
				return 0;
			}
		}
#endif // _PENWIN
		dti_lock(RIRCX(lpData)->dtiptr);
		//  if (((p_dti_descr_type)(RIRCX(lpData)->dtiptr))->p_pdf != _NULL) {
		//    if (((P_PDF_LOAD_HEADER)(((p_dti_descr_type)(RIRCX(lpData)->dtiptr))->
		//                                          p_pdf))->pMainHeader != _NULL) {
		/* Call postprocessing  see same code in rec-main.c DoRecognition */
		create_rwg_ppd(_NULL, RIRCX(lpData), RIXRDATA(lpData), RISYGRAPH(lpData));
		RIRCX(lpData)->trace = RITRACE(lpData) + ((rec_info_type _PTR)lpData)->done_to_point - 1;
		RIRCX(lpData)->ii = ((rec_info_type _PTR)lpData)->num_points_inrec;

#if USE_POSTRPOC
		EvaluateAndSortAnswers(RIRECWORD(lpData),
							   RIRCX(lpData),
							   RIXRDATA(lpData),
							   RISYGRAPH(lpData));
#else
		MakeAndCombRecWordsFromWordGraph(RISYGRAPH(lpData), RIRCX(lpData),
										 RIXRDATA(lpData),
										 RIRECWORD(lpData));
#endif

		PrintHeightPenalties(lpData);

		FreeRWGMem(RISYGRAPH(lpData));
		//    }
		//  }
		dti_unlock(RIRCX(lpData)->dtiptr);
#ifndef _PENWIN
		if (lpLocData->SpMode)
		{
			// restore old xr string
			Matrix.MPxr[wwc_pos + 1] = savedxr;
			Matrix.MPxr[wwc_pos + 2].xdxr.xnxr = save;
		}
#endif // _PENWIN
		return 0;
	} /* end of LayOutFunction */

	/* =============================================================== */
	_SHORT _Bfunction(p_CHAR word)
	{
#ifdef _PENWIN
		_SHORT        i, nl, x, y;
		ppd_type _PTR ppd_ptr;
		rec_w_type    rec_w[NUM_RW];
		xrvoc_type    xr;

		ppd_ptr = _NULL;

		strcpy(rec_w[0].word, word);
		rec_w[1].word[0] = 0;
		rec_w[0].weight = 200;

		LockDataFiles(rcm.datptr);

		create_ppd(&rcm, &xrdata, &rec_w, &ppd_ptr);
		if (ppd_ptr == _NULL)
		{
			UnlockDataFiles(rcm.datptr);
			return FALSE;
		}

		printw("\n");
		for (nl = 0; nl < w_lim && ppd_ptr[nl].letter != 0; nl ++)
		{
			printfw("%c", ppd_ptr[nl].letter);
			for (i = 0; i < XR_SIZE && ppd_ptr[nl].xr[i] != 0; i ++)
			{
				printfw(" ");
			}
		}
		printfw("\n");
		for (nl = 0; nl < w_lim && ppd_ptr[nl].letter != 0; nl ++)
		{
			for (i = 0; i < XR_SIZE && ppd_ptr[nl].xr[i] != 0; i ++)
			{
				xr.xr = ppd_ptr[nl].xr[i];
				xr.a  = ppd_ptr[nl].attr[i] & 0xf0;
				xr.h  = ppd_ptr[nl].attr[i] & 0x0f;
				xr.p = 0;
				put_xr(xr, 1);
			}
			printfw(" ");
		}
		printfw("\n");

		PPD_dealloc(&ppd_ptr);
		UnlockDataFiles(rcm.datptr);
#endif /* _PENWIN */
		return TRUE;
	}

	/* =============================================================== */

	_VOID dbgDrawInit(_INT left, _INT top, _INT right, _INT bottom)
	{
		DRAWINIT_TYPE di;
		DBGTEST;
		if (!ISDBG)
		{
			NODBG;
			return;
		}
		else
		{
			di.DImode = 0; /* reserved */
			di.DIview.left = left;
			di.DIview.top = top;
			di.DIview.right = right;
			di.DIview.bottom = bottom;
			di.DIbox.left = left;
			di.DIbox.top = top;
			di.DIbox.right = right;
			di.DIbox.bottom = bottom;
			DBGREQ(DBG_DRAWINIT, (p_DRAWINIT) &di);
			OpenTextWindow(1);
		}
	} /* end of dbgDrawInit */

	/* =============================================================== */

	_VOID dbgAddTrace(PS_point_type _PTR p_trace, _SHORT num_points)
	{
		TRACEDATA_TYPE trc;
		DBGTEST;
		if (!ISDBG)
		{
			NODBG;
			return;
		}
		else
		{
			trc.TDnp = num_points;
			trc.TDpt = (LPPOINT) p_trace;
			DBGREQ(DBG_ADDTRACE, (p_TRACEDATA) &trc);
		}
	} /* end of dbgAddTrace */

	/* =============================================================== */

	_VOID dbgAddRaster(p_ULONG lRaster, p_ULONG hRaster, _SHORT width)
	{
		RASTERDATA_TYPE rdata;
		DBGTEST;
		if (!ISDBG)
		{
			NODBG;
			return;
		}
		else
		{
			rdata.lRaster = lRaster;
			rdata.hRaster = hRaster;
			rdata.nWidth = width;
			rdata.nHeight = 32;
			DBGREQ(DBG_ADDRASTER, (p_RASTERDATA) &rdata);
		}
	} /* end of dbgAddRaster */

	/* =============================================================== */

	_VOID dbgColorTrace(_SHORT beg, _SHORT end, _SHORT color)
	{
		COLORRANGE_TYPE clr;
		DBGTEST;
		if (!ISDBG)
		{
			NODBG;
			return;
		}
		else
		{
			clr.color = GETWINCOLOR(color);
			clr.beg = beg;
			clr.end = end;
			DBGREQ(DBG_COLORTRACE, (p_COLORRANGE) &clr);
		}
	} /* end of dbgColorTrace */

	/* =============================================================== */
	_VOID dbgAddBox(_RECT box, _INT col, _INT col1, _INT style)
	{
		/*  drawes box, blinking if col != col1 */
		BOX_TYPE    box_loc;
		DBGTEST;
		if (!ISDBG)
		{
			NODBG;
			return;
		}
		else
		{
			box_loc.left = box.left;
			box_loc.right = box.right;
			box_loc.top = box.top;
			box_loc.bottom = box.bottom;
			box_loc.color = GETWINCOLOR(col);
			box_loc.color1 = GETWINCOLOR(col1);
			box_loc.style = style;
			DBGREQ(DBG_ADDDRAWBOX, (p_BOX_TYPE) &box_loc);
		}
	} /* dbgDrawBox */

#ifdef __cplusplus
	extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */
	/* =============================================================== */
	/* =============================================================== */
	/* =============================================================== */
#ifndef _PENWIN
	_INT DLLEXPORT RCBAccess(p_VOID pObject, /* root object Handle */
							 _INT nObjectId,     /* object id */
							 _INT nCommand,      /* command id */
							 _UINT uiParam,      /* multifunction uint parameter */
							 _ULONG ulParam)     /* multifunction ulong parameter */
	{
		if (pObject == _NULL)
		{
			return ERROR_BAD_OBJECT;
		}
		switch (nObjectId)
		{
			case OBJ_XRDATA:
					return XRDATA_access(pObject, nCommand, uiParam, ulParam);
			case OBJ_RCONTEXT:
					return RCONTEXT_access(pObject, nCommand, uiParam, ulParam);
			case OBJ_RECWORDS:
					return RECWORD_access(pObject, nCommand, uiParam, ulParam);
			case OBJ_TRAJECTORY:
					return TRACE_access(pObject, nCommand, uiParam, ulParam);
			case OBJ_RWGRAPH:
					return RWGRAPH_access(pObject, nCommand, uiParam, ulParam);
			case OBJ_BASELINE:
					return BASELINE_access(pObject, nCommand, uiParam, ulParam);
			case OBJ_RCCORRECTORDATA:
					return CORRECTORDATA_access(pObject, nCommand, uiParam, ulParam);
			default:
					return ERROR_BAD_OBJECT;
		}
		return ERROR_BAD_OBJECT;
	}

#ifdef __cplusplus
}            /* Assume C declarations for C++ */
#endif  /* __cplusplus */
/* =============================================================== */
/* =============================================================== */
/* =============================================================== */
_INT RECWORD_access(p_VOID pObject, /* root object Handle */
					_INT nCommand,      /* command id */
					_UINT uiParam,      /* multifunction uint parameter */
					_ULONG ulParam)     /* multifunction ulong parameter */
{
	rec_w_type _PTR   pin;
	p_LAB_RECWORD pex;
	_INT n, m;

	pin = (rec_w_type _PTR)(((rec_info_type _PTR)pObject)->prwords);
	pex = (p_LAB_RECWORD) ulParam;

	switch (nCommand)
	{
		case CMD_GET:
				if (pin == _NULL || pex == _NULL)
				{
					return ERROR_BAD_PARAM;
				}

			for (m = 0; m < (_INT) uiParam; m++)
			{
				for (n = 0; n < LAB_RW_SIZE && n < w_lim; n++)
				{
					pex->rwword[n] = pin->word[n];
					pex->rwlinp[n] = pin->linp[n];
					pex->rwnvar[n] = pin->nvar[n];
				}
				pex->rwweight = pin->weight;
				pex->rwsource = pin->src_id;
				pex->rwdattrib = pin->attr;
				if (pex->rwword[0] == 0)
				{
					break;
				}
				pex++;
				pin++;
			}
			return m;

		case CMD_SET:
				if (pin == _NULL || pex == _NULL)
				{
					return ERROR_BAD_PARAM;
				}
			for (m = 0; m < (_INT) uiParam; m++)
			{
				for (n = 0; n < LAB_RW_SIZE && n < w_lim; n++)
				{
					pin->word[n] = pex->rwword[n];
					pin->linp[n] = pex->rwlinp[n];
					pin->linp[n] = pex->rwnvar[n];
				}
				pin->weight = pex->rwweight;
				pin->src_id = pex->rwsource;
				pin->attr = pex->rwdattrib;
				if (pex->rwword[0] == 0)
				{
					break;
				}
				pex++;
				pin++;
			}
			return m;

		case CMD_INF:
				return NUM_RW;

		case CMD_NOP:
			default:
					return ERROR_BAD_CMD;
	}
}

/* =============================================================== */
/* =============================================================== */
/* =============================================================== */
_INT XRDATA_access(p_VOID pObject, /* root object Handle */
				   _INT nCommand,      /* command id */
				   _UINT uiParam,      /* multifunction uint parameter */
				   _ULONG ulParam)     /* multifunction ulong parameter */
{
	xrd_el_type _PTR pin;
	p_LAB_XRDATA pex;
	_INT m, xrlen;

	pin = (p_xrd_el_type) (((p_xrdata_type) (((rec_info_type _PTR)pObject)->pxrdata))->xrd);
	xrlen = (_INT) uiParam;
	pex = (p_LAB_XRDATA) ulParam;

	switch (nCommand)
	{
		case CMD_GET:
				if (pin == _NULL || pex == _NULL)
				{
					return ERROR_BAD_PARAM;
				}

			for (m = 0; m < xrlen && m < XRINP_SIZE; m++)
			{
				pex->xdxr.xnxr = pin->xr.type;
				pex->xdxr.xna = pin->xr.attrib;
				pex->xdxr.xnp = pin->xr.penalty;
				pex->xdxr.xnh = pin->xr.height;

				pex->xdxr.xns = pin->xr.shift;
				pex->xdxr.xno = pin->xr.orient;
				pex->xdxr.xnd = pin->xr.depth;

				pex->xdhotpt = pin->hotpoint;
				pex->xdbegpt = pin->begpoint;
				pex->xdendpt = pin->endpoint;
				pex->xdleft = pin->box_left;
				pex->xdtop = pin->box_up;
				pex->xdright = pin->box_right;
				pex->xdbottom = pin->box_down;

				if (pex->xdxr.xnxr == 0)
				{
					break;
				}

				pex++;
				pin++;
			}
			return m;

		case CMD_SET:
				if (pin == _NULL || pex == _NULL)
				{
					return ERROR_BAD_PARAM;
				}
			for (m = 0; m < xrlen && m < XRINP_SIZE; m++)
			{
				pin->xr.type = pex->xdxr.xnxr;
				pin->xr.attrib = pex->xdxr.xna;
				pin->xr.penalty = pex->xdxr.xnp;
				pin->xr.height = pex->xdxr.xnh;

				pin->xr.shift = pex->xdxr.xns;
				pin->xr.orient = pex->xdxr.xno;
				pin->xr.depth = pex->xdxr.xnd;

				pin->hotpoint = pex->xdhotpt;
				pin->begpoint = pex->xdbegpt;
				pin->endpoint = pex->xdendpt;
				pin->box_left = pex->xdleft;
				pin->box_up = pex->xdtop;
				pin->box_right = pex->xdright;
				pin->box_down = pex->xdbottom;

				if (pin->xr.type == 0)
				{
					break;
				}

				pex++;
				pin++;
			}
			((p_xrdata_type) (((rec_info_type _PTR)pObject)->pxrdata))->len = m;
			return m;

		case CMD_INF:
				return XRINP_SIZE;

		case CMD_NOP:
			default:
					return ERROR_BAD_CMD;
	}
}

/* =============================================================== */
/* =============================================================== */
/* =============================================================== */
_INT TRACE_access(p_VOID pObject, /* root object Handle */
				  _INT nCommand,      /* command id */
				  _UINT uiParam,      /* multifunction uint parameter */
				  _ULONG ulParam)     /* multifunction ulong parameter */
{
	PS_point_type _PTR pin;
	p_LAB_PT pex;
	_INT n, m;

	pin = (PS_point_type _PTR)((rec_info_type _PTR)pObject)->pTrace;
	pex = (p_LAB_PT) ulParam;

	switch (nCommand)
	{
		case CMD_GET:
				if (pin == _NULL || pex == _NULL)
				{
					return ERROR_BAD_PARAM;
				}

			pin += ((rec_info_type _PTR)pObject)->done_to_point - 1;
			for (m = 0, n = 0; m < (_INT) ((rec_info_type _PTR)pObject)->num_points_inrec - 1; m++)
			{
				pex->ptx = pin->x;
				pex->pty = pin->y;
				if (pex->pty == -1)
				{
					pex->ptx = n++;
				}
				pex++;
				pin++;
			}
			m++;
			pex->ptx = n;
			pex->pty = -1;
			return m;

		case CMD_SET:
				if (pin == _NULL || pex == _NULL)
				{
					return ERROR_BAD_PARAM;
				}

			for (m = 0, n = 0; m < (_INT) uiParam && m < MAX_BUF_POINTS - 32; m++)
			{
				pin->x = (_SHORT) pex->ptx;
				pin->y = (_SHORT) pex->pty;
				if (pin->y == -1)
				{
					pin->x = (_SHORT) n++;
				}
				pex++;
				pin++;
			}

			pin->x = (_SHORT) n;
			pin->y = -1;

			((rec_info_type _PTR)pObject)->num_points_got = m;
			((rec_info_type _PTR)pObject)->strokes_got = n;
			((rec_info_type _PTR)pObject)->done_to_point = 1;
			((rec_info_type _PTR)pObject)->num_points_inrec = 0;
			return m;

		case CMD_INF:
				return (_INT) ((rec_info_type _PTR)pObject)->num_points_inrec;

		case CMD_NOP:
			default:
					return ERROR_BAD_CMD;
	}
}

typedef struct _PAIR_TYPE
{
	p_VOID first;
	p_VOID second;
} PAIR_TYPE, _PTR p_PAIR;

/* =============================================================== */
/* =============================================================== */
/* =============================================================== */
_INT RWGRAPH_access(p_VOID pObject, /* root object Handle */
					_INT nCommand,      /* command id */
					_UINT uiParam,      /* multifunction uint parameter */
					_ULONG ulParam)     /* multifunction ulong parameter */
{
	RWG_type _PTR pin;
	p_GRAPH_TYPE pex;
	PAIR_TYPE pair;

	if (pObject == _NULL)
	{
		return ERROR_BAD_PARAM;
	}

	pin = RISYGRAPH((rec_info_type _PTR)pObject);
	pex = (p_GRAPH_TYPE) ulParam;

	switch (nCommand)
	{
		case CMD_GET:
				if (pin == _NULL || pex == _NULL)
				{
					return ERROR_BAD_PARAM;
				}

			pex->type = pin->type;
			pex->size = pin->size;
			if (pex->bafterpp)
			{
				pair.first = pex->prwse;
				pair.second = pex->pppde;
				RWGDATA_access(pObject, CMD_GET, uiParam, (_ULONG) &pair);
			}
			else
			{
				pair.first = pex->prwsb;
				pair.second = pex->pppdb;
				RWGDATA_access(pObject, CMD_GET, uiParam, (_ULONG) &pair);
			}
			return 1;

		case CMD_SET:
				if (pin == _NULL || pex == _NULL)
				{
					return ERROR_BAD_PARAM;
				}
			pin->type = pex->type;
			pin->size = pex->size;
			return RWGDATA_access(pObject, CMD_SET, uiParam, (_ULONG) pex->prwsb);

		case CMD_INF:
				return (_INT) pin->size;

		case CMD_NOP:
			default:
					return ERROR_BAD_CMD;
	}
}

/* =============================================================== */
/* =============================================================== */
/* =============================================================== */
_INT RWGDATA_access(p_VOID pObject, /* root object Handle */
					_INT nCommand,      /* command id */
					_UINT uiParam,      /* multifunction uint parameter */
					_ULONG ulParam)     /* multifunction ulong parameter */
{
	RWS_type _PTR pin;
	p_LAB_RWGNODE pex;
	RWG_PPD_el_type _PTR ppin;
	p_LAB_PPNODE ppex;
	_INT n, m;
	p_VOID dtp;
	xrp_type xrp[DTI_XR_SIZE + 1];

	if (pObject == _NULL)
	{
		return ERROR_BAD_PARAM;
	}

	pin = (p_RWS_type) RISYGRAPH((rec_info_type _PTR)pObject)->rws_mem;

	switch (nCommand)
	{
		case CMD_GET:
				pex = (p_LAB_RWGNODE) ((p_PAIR) ulParam)->first;
			dtp = RIRCX((rec_info_type _PTR)pObject)->dtiptr;
			if (pin == _NULL || pex == _NULL)
			{
				return ERROR_BAD_PARAM;
			}

			ppin = (RWG_PPD_el_type _PTR)RISYGRAPH((rec_info_type _PTR)pObject)->ppd_mem;
			ppex = ((p_LAB_PPNODE) ((p_PAIR) ulParam)->second);

			if (ppin == _NULL || ppex == _NULL)
			{
				return ERROR_BAD_PARAM;
			}

			for (m = 0; m < (_INT) uiParam; m++)
			{
				pex->rnasym = pin->sym;
				pex->rnrsym = pin->realsym;
				pex->rntype = pin->type;
				pex->rnvar = pin->nvar;
				pex->rnxbeg = pin->xrd_beg;
				pex->rnxlen = pin->xrd_len;
				pex->rnweight = pin->weight;
				pex->rnsrc = pin->src_id;
				pex->rnattr = pin->attr;
				pex->rnuser = pin->d_user;
				if (pin->type == RWST_SYM)
				{
					GetVarOfChar(pin->realsym, pin->nvar, &xrp[0], dtp);
				}
				else
				{
					HWRMemSet(&xrp[0], 0, sizeof(xrp_type)*(DTI_XR_SIZE + 1));
				}

				for (n = 0; n <= DTI_XR_SIZE; n++)
				{
#if DTI_COMPRESSED
					ppex->pnxr.xnxr= (_UCHAR)((xrp[n].type >> 5) | ((xrp[n].penl & 0xF0) >> 1));
					ppex->pnxr.xna = xrp[n].attr;
					ppex->pnxr.xnp = (_UCHAR)(xrp[n].penl & 0x0F);
					ppex->pnxr.xnh = (_UCHAR)(1);

					ppex->pnxr.xns = 0;
					ppex->pnxr.xno = 0;
					ppex->pnxr.xnd = 0;
#else
					ppex->pnxr.xnxr = xrp[n].type;
					ppex->pnxr.xna = xrp[n].attr;
					ppex->pnxr.xnp = xrp[n].penl;
					ppex->pnxr.xnh = xrp[n].height;

					ppex->pnxr.xns = 0;
					ppex->pnxr.xno = 0;
					ppex->pnxr.xnd = 0;
#endif

					ppex->pnalias = ppin->alias;
					ppex->pntype = ppin->type;
					ppex++;
					ppin++;
				}

				pex++;
				pin++;
			}
			return m;

		case CMD_SET:
				pex = (p_LAB_RWGNODE) ulParam;

			if (pex == _NULL)
			{
				return ERROR_BAD_PARAM;
			}

			if (pin == _NULL)
			{
				pin = (p_RWS_type) HWRMemoryAlloc(sizeof(RWS_type)*uiParam);
			}

			if (pin == _NULL)
			{
				return ERROR_BAD_PARAM;
			}

			RISYGRAPH((rec_info_type _PTR)pObject)->rws_mem = (p_VOID) pin;

			for (m = 0; m < (_INT) uiParam; m++)
			{
				pin->sym = pex->rnasym;
				pin->realsym = pex->rnrsym;
				pin->type = pex->rntype;
				pin->nvar = pex->rnvar;
				pin->xrd_beg = pex->rnxbeg;
				pin->xrd_len = pex->rnxlen;
				pin->weight = pex->rnweight;
				pin->src_id = pex->rnsrc;
				pin->attr = pex->rnattr;
				pin->d_user = pex->rnuser;
				pex++;
				pin++;
			}
			return m;

		case CMD_INF:
				return RWS_MAX_ELS;

		case CMD_NOP:
			default:
					return ERROR_BAD_CMD;
	}
}

/* =============================================================== */
/* =============================================================== */
/* =============================================================== */
_INT BASELINE_access(p_VOID pObject, /* root object Handle */
					 _INT nCommand,      /* command id */
					 _UINT uiParam,      /* multifunction uint parameter */
					 _ULONG ulParam)     /* multifunction ulong parameter */
{
	p_SHORT pin;
	p_SHORT pdst;
	p_TRACEDATA pex;
	_INT m;

	pin = (p_SHORT) RIRCX((rec_info_type _PTR)pObject)->curv_bord;
	pex = (p_TRACEDATA) ulParam;

	switch (nCommand)
	{
		case CMD_GET:
				if (pin == _NULL || pex == _NULL)
				{
					return ERROR_BAD_PARAM;
				}

			pex->TDnp = (_INT) uiParam;
			m = 0;
			if (pex->TDpt != NULL)
			{
				pdst = (p_SHORT) pex->TDpt;
				for (m = 0; m < (_INT) uiParam && m < CB_NUM_VERTEX; m++)
				{
					*pdst++ = *pin++;
				}
			}
			return m;

		case CMD_SET:
				return ERROR_BAD_CMD;

		case CMD_INF:
				return CB_NUM_VERTEX;

		case CMD_NOP:
			default:
					return ERROR_BAD_CMD;
	}
}

/* =============================================================== */
_INT RCONTEXT_access(p_VOID pObject, /* root object Handle */
					 _INT nCommand,      /* command id */
					 _UINT uiParam,      /* multifunction uint parameter */
					 _ULONG ulParam)     /* multifunction ulong parameter */
{
	rc_type _PTR pin;
	_INT _PTR    pex;

	pin = (rc_type _PTR)((rec_info_type _PTR)pObject)->prc;

	switch (nCommand)
	{
		case CMD_INF:
				pex = (_INT _PTR)ulParam;
			*pex = pin->caps_mode;
			pex++;
			*pex = pin->bad_amnesty;
			return 0;

		case CMD_VOC: /* uiParam==0: GetVoc, else: SetVoc */
			{
				p_VOID     *pvoc = (p_VOID *) ulParam;

				if (uiParam)
		{
			pin->vocptr[0] = (p_VOID) *pvoc;
			}
			else
			{
				*pvoc = (p_VOID) pin->vocptr[0];
			}
		return 0;
	}

		case CMD_NOP:
			default:
					return ERROR_BAD_CMD;
	}
}
/* =============================================================== */
//??SD 09/26 add
_INT CORRECTORDATA_access(p_VOID pObject, /* root object Handle */
						  _INT nCommand,      /* command id */
						  _UINT uiParam,      /* multifunction uint parameter */
						  _ULONG ulParam)     /* multifunction ulong parameter */
{
	p_rec_info_type  pin;
	_SHORT  _PTR     pData = (_SHORT _PTR)ulParam;

	pin = (rec_info_type _PTR)pObject;

	switch (nCommand)
	{
		case CMD_SET:
			{
				PS_point_type _PTR pTrace = (PS_point_type _PTR)((rec_info_type _PTR)pObject)->pTrace;
				_SHORT iBeg = (_SHORT) ((rec_info_type _PTR)pObject)->done_to_point;
				_SHORT iEnd = iBeg + (_SHORT) ((rec_info_type _PTR)pObject)->num_points_got - 1;
				_SHORT baseline = pData[0];
				_SHORT xHeight = pData[1];
				_SHORT yPeriod = pData[2];
				_SHORT y1 = baseline - xHeight;
				_SHORT y2 = baseline;
				_SHORT yy = (y1 + y2) / 2;
				_SHORT rec_y1 = -1;
				_SHORT rec_y2 = -1;
				_SHORT rec_yy;
				_SHORT i, n, dy0, dy1;

				for (i = 0; i<3; i++)
		{
			if (pData[i] <= 0)
				{
					return 0;
				}
			}
		for (i = iBeg; i <= iEnd; i++) if (pTrace[i].y >= 0)
		{
			if (pTrace[i].y < rec_y1 || rec_y1 == -1)
				{
					rec_y1 = pTrace[i].y;
				}
				if (pTrace[i].y > rec_y2 || rec_y2 == -1)
				{
					rec_y2 = pTrace[i].y;
				}
			}
		rec_yy = (rec_y1 + rec_y2) / 2;

				 for (dy0 = rec_yy>yy ? rec_yy - yy : yy - rec_yy, n = 0;; n++)
	{
		yy += yPeriod;
		dy1 = rec_yy > yy ? rec_yy - yy : yy - rec_yy;

		if (dy1 > dy0)
			{
				break;
			}

			dy0 = dy1;
		}
		y1 += yPeriod*n;
			  y2 += yPeriod*n;

			  pin->bb_top = y1;
			  pin->bb_bottom = y2;
			  return 0;
	}

		default:
				return ERROR_BAD_CMD;
	}
}
//??SD 09/26 end
/* =============================================================== */
/* =============================================================== */
#endif /* _PENWIN */

/* =============================================================== */
_INT IsXrLink(p_xrd_el_type xel);
_INT GetXrMovable(p_xrd_el_type xel); // Temp here until in xr_names ...
/* =============================================================== */
_INT  PrintHeightPenalties(p_rec_info_type pri)
{
	_INT i;
	_INT rwg_st;
	_INT yt, yb;
	_INT st, end;
	_INT pv, pb;
	_INT a, dy;
	_INT bx = RIRCX(pri)->stroka.box.left;
	p_RWS_type prwsc;
	p_RWG_type rwg = RISYGRAPH(pri);
	xrlw_ga_param_type pgap, gap = { 0 };
	p_xrdata_type  xrdata = RIXRDATA(pri);
	_RECT          box;

	printw("\nSym:Hsb:Chl -> ");

	dti_lock(RIRCX(pri)->dtiptr);
	prwsc = (p_RWS_type) rwg->rws_mem;
	rwg_st = 0;
	if (prwsc->type != RWST_SYM)
	{
		prwsc++;
		rwg_st++;
	}

	a = GetBaseBord(RIRCX(pri));

	for (i = rwg_st; i < rwg->size && prwsc->type == RWST_SYM; i++, prwsc++)
	{
		st = prwsc->xrd_beg;
		end = prwsc->xrd_beg + prwsc->xrd_len;

		if (GetSymBox(prwsc->realsym, st, end, xrdata, &box))
		{
			continue;
		}

		dy = a*(((box.right + box.left) / 2) - bx) / 128;

		yt = box.top - dy;
		yb = box.bottom - dy;

		gap.depth = (_UCHAR) i;
		//    gap.chlt  = chl_table[prwsc->realsym][prwsc->nvar];
		gap.chlt = GetVarPosSize(prwsc->realsym, prwsc->nvar, (p_dti_descr_type) RIRCX(pri)->dtiptr);
		gap.yt = (_SHORT) yt;
		gap.yb = (_SHORT) yb;

		//    gap.zt    = (_SHORT)yt;
		//    gap.zb    = (_SHORT)yb;
		gap.zt = (_SHORT) (yt + ((yb - yt)*((gap.chlt >> 0) & 0x000F)) / 16);
		gap.zb = (_SHORT) (yt + ((yb - yt)*(((gap.chlt >> 4) & 0x000F) + 1)) / 16);
		if (i == rwg_st)
		{
			pv = 0;
			pb = 0;
		}
		else
		{
			pv = 0; //GetCHLPenl(&pgap, &gap);
			pb = (pv >> 8) & 0xFF;
			pv &= 0xFF;
			pv -= pb;
		}

		printw("%c:%d:%d ", prwsc->realsym, pv, pb);

		pgap = gap;
	}

	printw(" ***  a = %d", a);

	dti_unlock(RIRCX(pri)->dtiptr);

	return 0;
}
#endif /* PG_DEBUG */
