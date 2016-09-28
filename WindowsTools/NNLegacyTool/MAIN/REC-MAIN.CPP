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

#define _REQ_WIN
#define STRICT
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#ifndef _FLAT32
#define _FLAT32
#endif /* _FLAT32 */
#endif /* _WIN32 */

#include "ams_mg.h"
#include "zctype.h"
#include "wgdtl.h"
//CHE DBG:

#if  PG_DEBUG && _CHE_
#include "\util\gest_pg\gest_pg\gest_pg.h"
#endif
/* ********************************************************* */
/* * Local defines                                         * */
/* ********************************************************* */

#define DEBUG_LEARN OFF

#define CMP_CONVERT_FROM_MAC 1

#include "ws.h"
#include "elk.h"

#include "lowlevel.h"
#include "xrword.h"

#if USE_ORTOGRAPH
#include "orto.h"
#include "nwtrec.h"
#include "nwt_db.h"
_CHAR     szLearnDataBase[256];
#endif /* USE_ORTOGRAPH */

// Eric:
#if USE_REJECT
#include "reject.h"
// Next define should be equal to one in wintest.h (wintester project):
#define DES_REJECT_MARK 0x8000
static _BOOL bReject;
#endif /* USE_REJECT */

#if PG_DEBUG
#include "pg_debug.h"
#endif /* PG_DEBUG */

#include "rec-main.h"

#include "wg_stuff.h"
#include "postcalc.h"
#include "pdf_file.h"
#include "psini.h"

#include "ldbutil.h"
#include "xrwstat.h" // OSO matrix
#include "xr_exp.h"

#include "recutil.h"
#include "main_utl.h"
#include "dti_lrn.h"
#include "pegrec.h"

enum
{
	PDB_,
	TDB_,
	DDB_,
	LDB_
};
/* ********************************************************* */
/* * Global and variables                                  * */
/* ********************************************************* */

xrdata_type _PTR   xrdata; /* used in postcalc.c */
rc_type            rcm;
p_rec_info_type    prig;

ws_word_info_type wswi = { 0 };

/* Global trace for pass to the postprocessing. */
#if  USE_POSTPROC
char               PDFNameBuffer[60];
PDF_LOAD_HEADER    PDFHeader = { _NULL, _NULL, 0};
#endif /* USE_POSTPROC */

#if PG_DEBUG
_INT               look;
_INT               g_write_des = 1;
#endif /* PG_DEBUG */

_UCHAR             fl_cut = 0;

//testing RAM DTE:
_CHAR    szRAMDtiName[128] = "";
//testing learning:
//_CHAR    szLrmName[LRM_FNAME_LEN] = "";
_BOOL    bLoadLRM = _FALSE;
_BOOL    bSaveLRM = _FALSE;

ws_control_type    wsc; // = {0};
/* ********************************************************* */
/* * end of global variables                               * */
/* ********************************************************* */


#if USE_CHUNK_PROCESSOR
#include "_postch.h"
#endif /* USE_CHUNK_PROCESSOR */

/* *************************************************************** */
/* *************************************************************** */
/* *************************************************************** */

p_CHAR ver_h =
#if defined(FOR_GERMAN)
    "32 bit WritePad recognizer ver. 14.30 German";
#elif defined(FOR_FRENCH)
    "32 bit WritePad recognizer ver. 14.30 French";
#else /* FOR_GERMAN... */
    "32 bit WritePad recognizer ver. 14.30 English";
#endif /* FOR_GERMAN... */

/* *************************************************************** */
/* *************************************************************** */
/* *************************************************************** */

extern xrdata_type _PTR xrdata;

/* *************************************************************** */
/* *                                                             * */
/* *************************************************************** */

//CHE: experiment
void  EstimateUserProblems(rec_w_type *rec_words,
                           p_CHAR  pCMP,
                           rc_type _PTR rc,
                           xrdata_type _PTR xrd,
                           PS_point_type _PTR pTrace,
                           _SHORT nPoints);
_INT RecognizeNoGroup(rec_info_type _PTR pri);
_INT EmulateWritingAndRecognize(rec_info_type _PTR pri);
_INT SegmAndRecognize(rec_info_type _PTR pri);
_INT PrepareRecInfo(rec_info_type _PTR pri);
_INT UnPrepareRecInfo(rec_info_type _PTR pri);

_INT RecSubTrace(rec_info_type _PTR pri);

_INT InitRecInfo(_UCHAR _PTR psinifilename, rec_info_type _PTR pri);
_INT LoadDtes(rec_info_type _PTR pri);
_INT LoadVocs(rec_info_type _PTR pri);

_INT PegRecWord(rec_info_type _PTR pri);
_INT PegRecBunch(rec_info_type _PTR pri);
p_CGR_control_type PegInitControlData(rec_info_type _PTR pri);

_SHORT  ini_fileGET(_HFILE fini,
                    p_UCHAR varname,
                    _UCHAR type,
                    p_VOID receive_buf,
                    _SHORT buf_len);

_INT parse_ini_string(_UCHAR _PTR string,
                      _UCHAR _PTR keyword,
                      _UCHAR _PTR value,
                      _UCHAR _PTR comment);

_BOOL FindIniFile(p_UCHAR buffer, _SHORT bufsize);
_ULONG TimeTicks(_VOID);

#if MEMORY_DEBUG_ON
int count;
_VOID QueryBlocks(_VOID);
#endif /* MEMORY_DEBUG_ON */

#if DEBUG_LEARN
_SHORT DumpVex (_VOID _PTR dtePtr, _SHORT count, _UCHAR _PTR word);
#endif /* DEBUG_LEARN */

_INT  RecYield(p_VOID);

/* *************************************************************** */
/* *                                                             * */
/* *************************************************************** */
/* *************************************************************** */
/* *                                                             * */
/* *************************************************************** */

/* *************************************************************** */
#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */

/* *************************************************************** */
/* *                                                             * */
/* *************************************************************** */
_INT DLLEXPORT InitRecognition(_UCHAR _PTR psinifilename,rec_info_type _PTR pri)
{
	_INT result = _FALSE;
	_UCHAR filename[MAX_LINELEN];

#if MEMORY_DEBUG_ON
#if MEMORY_DEBUG_REPORT
	DPrintf5("Init recognition.\n");
#endif
	count = 0;
#endif /* MEMORY_DEBUG_ON */

	if (psinifilename == _NULL)
	{
#if PG_DEBUG
		printw("Init -- no ini specified\n");

#endif /* PG_DEBUG */
		if (FindIniFile((p_UCHAR) filename, MAX_LINELEN))
		{
			psinifilename = &filename[0];
		}
		else
		{
			return 0;
		}
	}

#if PG_DEBUG
	else
	{
		printw("Init %s with: %s\n", ver_h, psinifilename);
	}

#endif /* PG_DEBUG */
	if (pri != _NULL)
	{
		if (PrepareRecInfo(pri) != 0)
		{
			if (InitRecInfo(psinifilename, pri) != 0)
			{
				result = _TRUE;
			}
			else
			{
				UnPrepareRecInfo(pri);
			}
		}
	}
#if PG_DEBUG
	if (result)
	{
		printw("Init done\n");
	}
	else
	{
		err_msg("!Init failed!\n");
	}
#endif /* PG_DEBUG */

#if MEMORY_DEBUG_REPORT && MEMORY_DEBUG_ON
	DPrintf5("Init recognition finished.\n");
#endif /* MEMORY_DEBUG_ON */


	wsc.flags |= WS_FL_CLOSE;
	WordStrokes(_NULL, &wsc, _NULL);

	TestDTLTemplate(pri);
	return result;
}

/* *************************************************************** */
/* *                                                             * */
/* *************************************************************** */

_INT DLLEXPORT CloseRecognition(rec_info_type _PTR pri)
{

#if PG_DEBUG
	//  XrwStatDumpData();                     //OSO /*  Dump correlation statistics.  */
	xr_exp(_NULL, _NULL, _NULL, _NULL);

	printw("Close Recognition\n");
#endif /* PG_DEBUG */

	if (pri)
	{
		if (RIRCX(pri) != _NULL)
		{
			if (RIRCX(pri)->vocptr[0] != _NULL)
			{
				voc_unload(&(RIRCX(pri)->vocptr[0]));
				RIRCX(pri)->vocptr[0] = _NULL;
			}
			if (RIRCX(pri)->vocptr[1] != _NULL)
			{
				voc_unload(&(RIRCX(pri)->vocptr[1]));
				RIRCX(pri)->vocptr[1] = _NULL;
			}
			if (RIRCX(pri)->vocptr[2] != _NULL)
			{
				voc_unload(&(RIRCX(pri)->vocptr[2]));
				RIRCX(pri)->vocptr[2] = _NULL;
			}
			if (RIRCX(pri)->tr_ptr != _NULL)
			{
				triads_unload(&(RIRCX(pri)->tr_ptr));
				RIRCX(pri)->tr_ptr = _NULL;
			}
			if (RIRCX(pri)->dtiptr != _NULL)
			{
				dti_unload(&(RIRCX(pri)->dtiptr));
				RIRCX(pri)->dtiptr = _NULL;
			}
			if (RIRCX(pri)->cflptr != _NULL)
			{
				LdbUnload(&(RIRCX(pri)->cflptr));
				RIRCX(pri)->cflptr = _NULL;
			}
#if  USE_ORTOGRAPH
			if (RIRCX(pri)->fOrto || RIRCX(pri)->pDB!=_NULL)
			{
				if(RIRCX(pri)->fOrto & kORTO_LEARN)
				{
					SaveDataBase((p_UCHAR)szLearnDataBase,RIRCX(pri)->pDB);
				}
				DestroyDataBase(RIRCX(pri)->pDB);
				RIRCX(pri)->pDB =_NULL;
			}
#endif /* USE_ORTOGRAPH */
		}
		UnPrepareRecInfo(pri);

#if MEMORY_DEBUG_ON
		QueryBlocks();

#if MEMORY_DEBUG_REPORT
		DPrintf5("Close recognition finished.\n");
#endif

#endif /* MEMORY_DEBUG_ON */
	}

	return 1;
}

/* *************************************************************** */
/* * * MAIN RECOGNITION CALL *********************************** * */
/* *************************************************************** */
_INT DLLEXPORT DoRecognition(rec_info_type _PTR pri)
{
	_INT result;

	prig = pri;                      // Set current value of pri to a global variable

	dti_lock(RIRCX(pri)->dtiptr);
	triads_lock(RIRCX(pri)->tr_ptr);
	LockVocabularies((vocptr_type _PTR)RIRCX(pri)->vocptr[0]);

	pri->fl_cut = fl_cut;
	if (pri->fl_cut)
	{
		result = EmulateWritingAndRecognize(pri);
	}
	else
	{
		result = RecognizeNoGroup(pri);
	}

	UnlockVocabularies((vocptr_type _PTR)RIRCX(pri)->vocptr[0]);
	triads_unlock(RIRCX(pri)->tr_ptr);
	dti_unlock(RIRCX(pri)->dtiptr);
	return result;
}

/* *************************************************************** */
#ifdef __cplusplus
}            /* Assume C declarations for C++ */
#endif  /* __cplusplus */

/* *************************************************************** */
/* *************************************************************** */
_INT PrepareRecInfo(rec_info_type _PTR pri)
{
	if (pri == _NULL)
	{
		return 0;
	}

	pri->cmp_word = &pri->cmp_word_place[0];//KRAV
	if (RIRCX(pri) == _NULL)
	{
		VRIRCX(pri) = HWRMemoryAlloc(sizeof(rc_type));
		if (RIRCX(pri) == _NULL)
		{
			goto err;
		}
		HWRMemSet(VRIRCX(pri), 0, sizeof(rc_type));
	}

	if (RITRACE(pri) == _NULL)
	{
		VRITRACE(pri) = HWRMemoryAlloc(MAX_BUF_POINTS*sizeof(PS_point_type));
		if (RITRACE(pri) == _NULL)
		{
			goto err;
		}
		HWRMemSet(VRITRACE(pri), 0, MAX_BUF_POINTS*sizeof(PS_point_type));
	}

	if (RIXRDATA(pri) == _NULL)
	{
		pri->pxrdata = (p_xrdata_type) HWRMemoryAlloc(sizeof(xrdata_type));
		//RIXRDATA(pri) = (p_xrdata_type)HWRMemoryAlloc(sizeof(xrdata_type));
		if (RIXRDATA(pri) == _NULL)
		{
			return 0;
		}
		AllocXrdata(RIXRDATA(pri), XRINP_SIZE);
		if (RIXRDATA(pri)->size == 0)
		{
			return 0;
		}
	}

	if (RIRECWORD(pri) == _NULL)
	{
		VRIRECWORD(pri) = HWRMemoryAlloc(NUM_RW*sizeof(rec_w_type));
		if (RIRECWORD(pri) == _NULL)
		{
			goto err;
		}
		HWRMemSet(VRIRECWORD(pri), 0, NUM_RW*sizeof(rec_w_type));
	}

	if (RIMAINBUF(pri) == _NULL)
	{
		VRIMAINBUF(pri) = HWRMemoryAlloc(sizeof(DES_DATA_TYPE));
		if (RIMAINBUF(pri) == _NULL)
		{
			goto err;
		}
		HWRMemSet(VRIMAINBUF(pri), 0, sizeof(DES_DATA_TYPE));
	}

	if (RIALIAS(pri) == _NULL)
	{
		VRIALIAS(pri) = HWRMemoryAlloc(sizeof(ALS_NODE_TYPE));
		if (RIALIAS(pri) == _NULL)
		{
			goto err;
		}
		HWRMemSet(VRIALIAS(pri), 0, sizeof(ALS_NODE_TYPE));
	}

	if (RISYGRAPH(pri) == _NULL)
	{
		VRISYGRAPH(pri) = HWRMemoryAlloc(sizeof(RWG_type));
		if (RISYGRAPH(pri) == _NULL)
		{
			goto err;
		}
		HWRMemSet(VRISYGRAPH(pri), 0, sizeof(RWG_type));
	}

	return 1;
err:
#if PG_DEBUG
	err_msg("!Failed to alloc initial memory!\n");
#endif /* PG_DEBUG */
	return 0;
}

/* *************************************************************** */
_INT UnPrepareRecInfo(rec_info_type _PTR pri)
{
	if (pri == _NULL)
	{
		return 0;
	}

	if (RISYGRAPH(pri) != _NULL)
	{
		HWRMemoryFree(RISYGRAPH(pri));
		VRISYGRAPH(pri) = _NULL;
	}

	if (RIALIAS(pri) != _NULL)
	{
		HWRMemoryFree(RIALIAS(pri));
		VRIALIAS(pri) = _NULL;
	}

	if (RIMAINBUF(pri) != _NULL)
	{
		HWRMemoryFree(RIMAINBUF(pri));
		VRIMAINBUF(pri) = _NULL;
	}

	if (RIRECWORD(pri) != _NULL)
	{
		HWRMemoryFree(RIRECWORD(pri));
		VRIRECWORD(pri) = _NULL;
	}

	if (RIXRDATA(pri) != _NULL)
	{
		FreeXrdata(RIXRDATA(pri));
		HWRMemoryFree(RIXRDATA(pri));
		VRIXRDATA(pri) = _NULL;
	}

	if (RITRACE(pri) != _NULL)
	{
		HWRMemoryFree(RITRACE(pri));
		VRITRACE(pri) = _NULL;
	}
	if (RIRCX(pri) != _NULL)
	{
		HWRMemoryFree(RIRCX(pri));
		VRIRCX(pri) = _NULL;
	}

	return 1;
}

/* *************************************************************** */
/* *  Function: RecSubTrace                                      * */
/* *                                                             * */
/* *  Parameters:                                                * */
/* *    rec_info_type _PTR pointer to the recognizer information * */
/* *    rc_type _PTR pointer to the recognition control struct   * */
/* *                                                             * */
/* *  History:                                                   * */
/* *    12/21/93 created <bigor>                                 * */
/* *************************************************************** */
_INT RecSubTrace(rec_info_type _PTR pri)
{
	_INT             i;
	_ULONG           time, word_time, tpl_time, chunk_time, ppd_time, low_time, upper_time, orto_time;
	_BOOL            bWasXrdataChanged = _FALSE;
	_INT             mword;
	_INT             restore = 0;
	_INT             size, pos, nl;
	RCB_inpdata_type rcb = { 0 };
	p_rc_type        prc = RIRCX(pri);
	p_rc_type        prcl = RIRCX(pri);
	_BOOL            fErr = _FALSE;
	_INT    iStep;      /* CHE: iStep>0 may be used only in some rare cond- */
	/*    tions like discrepancy between border type   */
	/*    chosen and the answer after upper level.     */

#if USE_CHUNK_PROCESSOR
	p_VOID pctx = _NULL;
#endif /* USE_CHUNK_PROCESSOR */
	if (RITRACE(pri) == _NULL || RIRCX(pri) == _NULL ||
	        RIXRDATA(pri) == _NULL || RIRECWORD(pri) == _NULL ||
	        RIMAINBUF(pri) == _NULL)
	{
		return UNSUCCESS;
	}
#if USE_CHUNK_PROCESSOR
	ChunkAllocCtx(&pctx, RIRCX(pri));
#endif /* USE_CHUNK_PROCESSOR */
	(RIXRDATA(pri))->len = 0;
	HWRMemSet(RIRECWORD(pri), 0, NUM_RW*sizeof(rec_w_type));
	HWRMemSet(RIMAINBUF(pri), 0, sizeof(DES_DATA_TYPE));

#if PG_DEBUG
	/* indicate pen laboratory begining of the group
	of the strokes recognition */
#if MEMORY_DEBUG_ON && MEMORY_DEBUG_REPORT
	DPrintf5("word %d begin.\n", count);
#endif /* MEMORY_DEBUG_ON */

	DebugRequest(DBG_WORDBEG, 0L);
	/* collect trace data for use in pen laboratory */
	/* trace data without distortions */
	DbgFillTraceData(RITRACE(pri) + pri->done_to_point - 1, pri->num_points_inrec);
#endif

	upper_time = low_time = ppd_time = chunk_time = orto_time = 0l;
	RIMAINBUF(pri)->time_upper = 0l;
	RIMAINBUF(pri)->time_low = 0l;
	iStep = 0;

#if USE_CHUNK_PROCESSOR
	time = TimeTicks ();
	ChunkProcessor(pctx, RITRACE(pri)+pri->done_to_point-1, pri->num_points_inrec);
	ChunkModifyRC(pctx, RIRCX(pri));
	chunk_time += TimeTicks () - time;
#endif /* USE_CHUNK_PROCESSOR */
	prcl->trace = _NULL; /*CHE: Init "trace" field for PPD: */

#if PG_DEBUG
	RIXRDATA(pri)->len = 0;   /* request to analyze xrdata from pen laboratory */
	DebugRequest(DBG_PASSBEGIN, MAKELPARAM(iStep, 0));
	DebugRequest(DBG_READXRDATA, (_LONG) (p_VOID) RIXRDATA(pri));
	if (RIXRDATA(pri)->len == 0) /* check request to analyze xrdata from pen laboratory */
	{
		bWasXrdataChanged = _FALSE;    /* no request to analyze xrdata from pen laboratory */
	}
	else
	{
		bWasXrdataChanged = _TRUE;    /* was request to analyze xrdata from pen laboratory */
	}
	if (!bWasXrdataChanged)
	{
#endif /* PG_DEBUG */



		// ------------- Preprocess trajectory ------------------------

		prcl->trace = RITRACE(pri) + pri->done_to_point - 1;

		PreprocessTrajectory(prcl);

		// ------------- Set basic data for Stroka --------------------

		rcb.trace = RITRACE(pri) + pri->done_to_point - 1;
		rcb.num_points = pri->num_points_inrec;

		//      if (pri->num_word  == 0)
		if (1 || pri->num_word == 0)
		{
			rcb.flags |= (_SHORT) (RCBF_NEWAREA);
			rcb.prv_size =
			    rcb.prv_dn_pos =
			        rcb.prv_size_sure =
			            rcb.prv_pos_sure = 0;
		}
		else
		{
			rcb.prv_size = prc->stroka.size_out;
			rcb.prv_dn_pos = prc->stroka.dn_pos_out;
			rcb.prv_size_sure = prc->stroka.size_sure_out;
			rcb.prv_pos_sure = prc->stroka.pos_sure_out;
		}

		rcb.flags |= (_SHORT) RCBF_PREVBORD;

		if (GetWSBorder(pri->num_word, (p_ws_results_type) pri->p_word_strokes, &size, &pos, &nl) == 0)
		{
			rcb.ws_size = (_SHORT) size;
			rcb.ws_dn_pos = (_SHORT) pos;
			rcb.flags |= (_SHORT) RCBF_WSBORD;
			if (nl)
			{
				rcb.flags |= (_SHORT) (RCBF_NEWLINE);
			}
		}

		if (pri->fl_bedit_border)
		{
			rcb.prv_size = (_SHORT) (pri->bb_bottom - pri->bb_top);
			rcb.prv_dn_pos = (_SHORT) (pri->bb_bottom);
			rcb.prv_size_sure = 75;
			rcb.prv_pos_sure = 75;
			rcb.flags = (_SHORT) RCBF_PREVBORD;

			pri->fl_bedit_border = 0; //so as not to set LMOD_BOX_EDIT

			/*
			rcb.bx_size        = (_SHORT)(pri->bb_bottom - pri->bb_top);
			rcb.bx_dn_pos      = (_SHORT)(pri->bb_bottom);
			rcb.flags         |= (_SHORT)RCBF_BOXBORD;
			*/
		}

		SetRCB(&rcb, &(prc->stroka));


		// ------------- Get segmentation slope -----------------------

		{
			// experiments on transfer slope data from segmentation
			p_ws_results_type  wstr;
			p_word_strokes_type pws;

			wstr = (p_ws_results_type) prig->p_word_strokes;
			if (wstr)
			{
				pws = &((*wstr->pwsa)[prig->num_word]);

				prcl->slope = (_SHORT) ((pws->slope == 0) ? 1 : pws->slope);
			}
			else
			{
				prcl->slope = 0;
			}

			//prcl->slope = 0;
		}

		// ------------- Low level start ------------------------------

		time = TimeTicks();

#if  PG_DEBUG && _CHE_
		{
			GESTURE_TYPE  gt;
			p_CHAR        pMsg = "";

			gt = GestureCheck( GEST_ALL, (LPPOINTS)(RITRACE(pri) + pri->done_to_point-1),
			                   ((rc_type _PTR)prcl)->ii, 1 );

			switch(  gt  )
			{
				case    GEST_NONE:
					pMsg = "\n============= GEST_NONE =============\n";
					break;
				case    GEST_ERASE:
					pMsg = "\n============= GEST_ERASE =============\n";
					break;
				case    GEST_CAPITAL:
					pMsg = "\n============= GEST_CAPITAL =============\n";
					break;
				case    GEST_BACK:
					pMsg = "\n============= GEST_BACK =============\n";
					break;
				case    GEST_SPACE:
					pMsg = "\n============= GEST_SPACE =============\n";
					break;
				case    GEST_RETURN:
					pMsg = "\n============= GEST_RETURN =============\n";
					break;
				case    GEST_CORRECT:
					pMsg = "\n============= GEST_CORRECT =============\n";
					break;
				case    GEST_SPELL:
					pMsg = "\n============= GEST_SPELL =============\n";
					break;
				case    GEST_SELECTALL:
					pMsg = "\n============= GEST_SELECTALL =============\n";
					break;
				case    GEST_UNDO:
					pMsg = "\n============= GEST_UNDO =============\n";
					break;
				case    GEST_SMALLPT:
					pMsg = "\n============= GEST_SMALLPT =============\n";
					break;
				case    GEST_COPY:
					pMsg = "\n============= GEST_COPY =============\n";
					break;
				case    GEST_CUT:
					pMsg = "\n============= GEST_CUT =============\n";
					break;
				case    GEST_PASTE:
					pMsg = "\n============= GEST_PASTE =============\n";
					break;
				case    GEST_TAB:
					pMsg = "\n============= GEST_TAB =============\n";
					break;
				case    GEST_MENU:
					pMsg = "\n============= GEST_MENU =============\n";
					break;

				default:
					pMsg = "\n============= Unknown Gesture code!!! =============\n";
					break;
			}

			err_msg( pMsg );

		}
#endif /*PG_DEBUG*/


		/* call low-level trace analysis */
		if (low_level(RITRACE(pri) + pri->done_to_point - 1,
		              (xrdata_type _PTR)RIXRDATA(pri),
		              (rc_type _PTR)prcl) != SUCCESS)
		{
			fErr = _TRUE;
			goto error_in_rec;
		}
		/* measure low level analysis time */
		low_time += TimeTicks() - time;

		(*prcl->pFuncYield)(0);

#if PG_DEBUG
	}
#endif /* PG_DEBUG */

#if USE_CHUNK_PROCESSOR
	time = TimeTicks ();
	ChunkWriteParamCtx(pctx, RIRCX(pri), RIXRDATA(pri), RIRECWORD(pri));
	ChunkPatchXrdata(pctx);
	chunk_time += TimeTicks () - time;
#endif /* USE_CHUNK_PROCESSOR */

#if USE_POSTPROC && POSTCALC_FILE_RESULTS
	//      unlink(PRSFileName);
#endif /* USE_POSTPROC && POSTCALC_FILE_RESULTS */

	if (!bWasXrdataChanged)
	{
		RIRCX(pri)->xrw_mode &= ~(XRWM_MWORD);
		mword = SetMultiWordMarksDash(RIXRDATA(pri));
		mword += SetMultiWordMarksWS(RIRCX(pri)->lrn_min_class, RIXRDATA(pri), RIRCX(pri));
	}
	else
	{
		mword = 1;
	} /* GIT - can't check mword using functions above because of xredit
				   changes. If you'd like it to work, make all preparations manually */
	if (mword)
	{
		RIRCX(pri)->xrw_mode |= XRWM_MWORD;
	}

#if PG_DEBUG
	DbgFillXRData(RIXRDATA(pri)); /* collect xrdata for use in pen laboratory */
#endif /* PG_DEBUG */

	/* ----------------- Upper Level Start -------------------------------- */
#if PG_DEBUG
	if (mpr <= 0 || mpr >= 100 || mpr == 21)
	{
		/* store cmp-word from tap file in rec_words[0] if any for use in
		pen laboratory's postprocessing analysis */
		(*(rec_w_type(_PTR)[NUM_RW])RIRECWORD(pri))[0].word[0] = 0;
		DbgGetCMPWord((rec_w_type(_PTR)[NUM_RW])RIRECWORD(pri));
		if ((*(rec_w_type(_PTR)[NUM_RW])RIRECWORD(pri))[0].word[0] != 0)
		{
			HWRStrCpy((_STR) pri->cmp_word, (_STR) (*(rec_w_type(_PTR)[NUM_RW])RIRECWORD(pri))[0].word);
			if (pri->cmp_word[0] == '#')
			{
				pri->cmp_word[0] = 0;
			}
			if (pri->cmp_word[0] == ']')
			{
				HWRStrCpy((_STR) pri->cmp_word, (_STR) ((*(rec_w_type(_PTR)[NUM_RW])RIRECWORD(pri))[0].word + 2));
			}
#if CMP_CONVERT_FROM_MAC
			for (int s, x = 0; x < 32 && pri->cmp_word[x] != 0; x++)
				if ((s = MacToOS(pri->cmp_word[x])) != 0)
				{
					pri->cmp_word[x] = (_UCHAR) s;
				}
				else
				{
					pri->cmp_word[0] = 0;    // Can't convert from MAC
					break;
				}
#endif
		}
		else
		{
			pri->cmp_word[0] = 0;
		}

		HWRStrCpy((_STR) (*(rec_w_type(_PTR)[NUM_RW])RIRECWORD(pri))[0].word, (_STR) pri->cmp_word);
		(*(rec_w_type(_PTR)[NUM_RW])RIRECWORD(pri))[0].weight = 100;
		(*(rec_w_type(_PTR)[NUM_RW])RIRECWORD(pri))[1].weight = 0;
		(*(rec_w_type(_PTR)[NUM_RW])RIRECWORD(pri))[1].word[0] = 0;
#endif /* PG_DEBUG */

		time = TimeTicks();
		xrdata = RIXRDATA(pri);
		i = xrdata->len;
		if (i >= 3) // Cut off idle calls after Chunks or LowLev Err
		{
			if (xrw_algs(RIXRDATA(pri),
			             (rec_w_type(_PTR)[NUM_RW])RIRECWORD(pri),
			             RISYGRAPH(pri),
			             RIRCX(pri)) != 0)
			{
				fErr = _TRUE;
				goto error_in_rec;
			}
		}
		/* measure upper level analysis time */
		upper_time += TimeTicks() - time;

		(*prcl->pFuncYield)(0);

#if PG_DEBUG
		if (mpr >= 100) printw("\nAnswer before p/p: \"%s\", %d.",
			                       RIRECWORD(pri)[0].word, RIRECWORD(pri)[0].weight);
	}
#endif /*PG_DEBUG*/

	/* ----------------- PostProcessing Start -------------------------------- */
#if PG_DEBUG
	if (mpr <= 0 || mpr >= 100 || mpr == 21)
#endif /* PG_DEBUG */

	{
		time = TimeTicks();


		if (((p_dti_descr_type) (RIRCX(pri)->dtiptr))->p_pdf != _NULL)
		{
			if (((P_PDF_LOAD_HEADER) (((p_dti_descr_type) (RIRCX(pri)->dtiptr))->p_pdf))->pMainHeader != _NULL)
			{
#if PG_DEBUG
				if (bWasXrdataChanged)
				{
					restore = ((rc_type _PTR)(pri->prc))->fl_post;
					((rc_type _PTR)(pri->prc))->fl_post = FALSE;
				}
#endif  /* PG_DEBUG */

#if USE_POSTPROC
				EvaluateAndSortAnswers(RIRECWORD(pri),
				                       RIRCX(pri),
				                       RIXRDATA(pri),
				                       RISYGRAPH(pri));
#endif

				MakeAndCombRecWordsFromWordGraph(RISYGRAPH(pri), RIRCX(pri),
				                                 RIXRDATA(pri),
				                                 RIRECWORD(pri));
				//CHE: experiment

#if USE_POSTPROC
				EstimateUserProblems( RIRECWORD(pri),
				                      _NULL,
				                      RIRCX(pri),
				                      RIXRDATA(pri),
				                      RITRACE(pri) + pri->done_to_point-1,
				                      pri->num_points_inrec );
#endif
			}
		}
		else
		{
			MakeAndCombRecWordsFromWordGraph(RISYGRAPH(pri), RIRCX(pri),
			                                 RIXRDATA(pri),
			                                 RIRECWORD(pri));
		}

		ppd_time += TimeTicks() - time;
	}

	(*prcl->pFuncYield)(0);

#if PG_DEBUG
#if CMP_CONVERT_FROM_MAC
	{
		int a, b;

		for (a = 0; a < NUM_RW; a++)
		{
			for (b = 0; b < w_lim; b++)
			{
				(*(rec_w_type(_PTR)[NUM_RW])RIRECWORD(pri))[a].word[b] =
				    (_UCHAR) OSToMac((*(rec_w_type(_PTR)[NUM_RW])RIRECWORD(pri))[a].word[b]);
			}
		}
	}
#endif // CMP_CONVERT_FROM_MAC

	if (mpr >= 100) printw("\nAnswer after p/p:  \"%s\", %d.",
		                       RIRECWORD(pri)[0].word, RIRECWORD(pri)[0].weight);
	DbgFillRWData((rec_w_type(_PTR)[NUM_RW])RIRECWORD(pri));
	//  DbgFillGRData(pRWG, (p_VOID)pRWS, (p_VOID)pNewPPDData, 1);
	DbgFillGRData(RISYGRAPH(pri), (p_VOID) RISYGRAPH(pri)->rws_mem, (p_VOID) RISYGRAPH(pri)->ppd_mem, 0);
	DbgFillGRData(RISYGRAPH(pri), (p_VOID) RISYGRAPH(pri)->rws_mem, (p_VOID) RISYGRAPH(pri)->ppd_mem, 1);
	if (bWasXrdataChanged)
	{
		((rc_type _PTR)(pri->prc))->fl_post = (_UCHAR) restore;
	}
#endif /* PG_DEBUG */

#if USE_ORTOGRAPH
	time = TimeTicks();
	PPLearn(pri->cmp_word, RIRECWORD(pri), RISYGRAPH(pri),
	        RIXRDATA(pri), RIRCX(pri), RITRACE(pri) + pri->done_to_point-1);

	orto_time += TimeTicks() - time;
#endif /* USE_ORTOGRAPH */

#if USE_CHUNK_PROCESSOR
	time = TimeTicks ();
	ChunkRestoreRC(pctx, RIRCX(pri));
	ChunkSortAnswers(pctx);
	ChunkCorrectByLexDB(pctx);
	chunk_time += TimeTicks () - time;
#endif /* USE_CHUNK_PROCESSOR */

	/* ----------------- Fly learning Start -------------------------------- */

	if (pri->cmp_word[0] != 0)
	{
#if DEBUG_LEARN
		static int count_lrn=0;
#endif /* DEBUG_LEARN */
		for (i = 0; i < NUM_RW && RIRECWORD(pri)[i].word[0] != 0; i++)
		{
			if (!strcmp(pri->cmp_word, (_STR) RIRECWORD(pri)[i].word))
			{
				if (prcl->fly_learn)
				{
					FlyLearn(prcl, &RIRECWORD(pri)[i]);
				}
#if DEBUG_LEARN
				DumpVex(prcl->dtiptr, count_lrn++, RIRECWORD(pri)[i].word);
#endif /* DEBUG_LEARN 11-11-93 bigor */
				break;
			}
		}
	}

#if PG_DEBUG
	/* collect baseline shape for use in the pen laboratory */
	DbgFillBorderData((p_USHORT) prcl->curv_bord, CB_NUM_VERTEX);
	DebugRequest(DBG_PASSEND, MAKELPARAM(iStep, 0));
#endif

	word_time = chunk_time + low_time + upper_time + ppd_time + orto_time;
	i = HWRStrLen((_STR) RIRECWORD(pri)[0].word);
	if (i > 0)
	{
		tpl_time = word_time / i;
	}
	else
	{
		tpl_time = 0l;
	}

#if PG_DEBUG
	printw("Word time: %d.%03ds, Tpl: %d.%03ds, Low: %d.%03ds, Up: %d.%03ds, PP: %d.%03ds, Ch: %d.%03ds, Or: %d.%03ds\n",
	       (_INT) (word_time / 1000), (_INT) (word_time % 1000),
	       (_INT) (tpl_time / 1000), (_INT) (tpl_time % 1000),
	       (_INT) (low_time / 1000), (_INT) (low_time % 1000),
	       (_INT) (upper_time / 1000), (_INT) (upper_time % 1000),
	       (_INT) (ppd_time / 1000), (_INT) (ppd_time % 1000),
	       (_INT) (chunk_time / 1000), (_INT) (chunk_time % 1000),
	       (_INT) (orto_time / 1000), (_INT) (orto_time % 1000)
	      );

error_in_rec:

	if (g_write_des)
	{
		((DES_DATA_TYPE _PTR)pri->pmbuf)->time_low += (_USHORT) (low_time);
		((DES_DATA_TYPE _PTR)pri->pmbuf)->time_upper += (_USHORT) (upper_time);
#if USE_CHUNK_PROCESSOR
		pri->pctx=pctx;
#else
		pri->pctx = _NULL;
#endif /* USE_CHUNK_PROCESSOR */
		if (bWasXrdataChanged)
		{
			restore = pri->multiply_des;
			pri->multiply_des = 0;
		}
		FillDesBuffers(pri);
		if (bWasXrdataChanged)
		{
			pri->multiply_des = (_UCHAR) restore;
		}

#if MEMORY_DEBUG_ON && MEMORY_DEBUG_REPORT
		DPrintf5("word %d end.\n", count);
		count++;
#endif /* MEMORY_DEBUG_ON */
	}

#endif /* PG_DEBUG */

#if USE_CHUNK_PROCESSOR
	ChunkRestoreRC(pctx, RIRCX(pri));
	ChunkCleanUp(&pctx);
#endif /* USE_CHUNK_PROCESSOR */
	FreeRWGMem(RISYGRAPH(pri));

	if (fErr)
	{
		prc->stroka.size_out = rcb.prv_size;
		prc->stroka.dn_pos_out = rcb.prv_dn_pos;
		prc->stroka.size_sure_out = rcb.prv_size_sure;
		prc->stroka.pos_sure_out = rcb.prv_pos_sure;
	}

	return SUCCESS;
}

/* *************************************************************** */
/* *    Init Rec Info                                            * */
/* *************************************************************** */
/* *************************************************************** */
#ifndef ERR_MSG_EMPMACRO

#define  SAVE_ERR_MSG  0

_VOID err_msg(_CHAR _PTR msg)
{
#if SAVE_ERR_MSG
	_HTEXT hf;
	extern _CHAR WorkingDirName[];
	_CHAR  ErrFileName[256];
#endif /* SAVE_ERR_MSG */
	p_UCHAR szErr = _NULL;
	_UCHAR szMsg[140];

#if  PG_DEBUG
	static  _BOOL  bShowMsgBox = _TRUE;
#endif /* PG_DEBUG */
	if (msg != _NULL)
	{
		sprintf((p_CHAR) szMsg, (p_CHAR)"ERR_MSG> %s\n", (p_CHAR) msg);
#if  PG_DEBUG
		printw((_STR) szMsg);
		if (bShowMsgBox  &&  msg[0] == '!')
		{
			if (MessageBox(_NULL, &(msg[1]), "Wg32 error", MB_OKCANCEL | MB_ICONSTOP) == IDCANCEL)
			{
				bShowMsgBox = _FALSE;
			}
		}
#endif /* PG_DEBUG */
	}

#if  SAVE_ERR_MSG
	lstrcpy(ErrFileName,WorkingDirName);
	lstrcat(ErrFileName,"err.tst");
	hf = HWRTextOpen(ErrFileName, HWR_TEXT_NORMAL, HWR_TEXT_APPEND);
	if (hf == _NULL)
	{
		szErr = "Cannot open ErrFile";
	}
	else
		if (msg != _NULL)
		{
			if (!HWRTextPutS((_STR)szMsg, hf))
			{
				szErr = "Error writing ErrFile";
			}
		}
	if  ( hf != _NULL )
	{
		HWRTextClose(hf);
	}
#endif  /*SAVE_ERR_MSG*/

#if  PG_DEBUG
	if (szErr)
	{
		printw((_STR) szErr);
	}
#endif /* PG_DEBUG */

}

#endif //#ifndef ERR_MSG_EMPMACRO

/* *************************************************************** */
/* *    Init Rec Info                                            * */
/* *************************************************************** */
_INT InitRecInfo(_UCHAR _PTR psinifilename, rec_info_type _PTR pri)
{
	_INT i;
	_UCHAR buffer[MAX_LINELEN];
	_HFILE inif;

	if (pri == _NULL || psinifilename == _NULL)
	{
		return 0;
	}

	if ((inif = HWRTextOpen((_STR) psinifilename, HWR_TEXT_RDONLY, HWR_TEXT_EXCL)) == _NULL)
	{
#if PG_DEBUG
		printw("Failed to open INI file: %s!\n", psinifilename);
		err_msg("!Failed to open INI file");
#endif /* PG_DEBUG */
		return 0;
	}

	//CHE: experiment
	{
		_USHORT  fl_shake;
		const char* str = "Shake border";
		if (ini_fileGET(inif, (p_UCHAR) str, 'I',
		                &fl_shake, 0) != 0)
		{
			fl_shake = 0;
		}
		RIRCX(pri)->fl_fil = (_BOOL) fl_shake;
	}
	/* set recoghition mode */
	if (ini_fileGET(inif, Recognition_mode, 'S',
	                buffer, PARA * 8) != 0)
	{
		HWRStrCpy((_STR) buffer, (_STR) modetext);
	}
#if USE_FRM_WORD
	RIRCX(pri)->rec_mode = 0;
	for (i = 0; buffer[i] != 0; i++)
	{
		switch (buffer[i])
		{
			case  't':
			case  'T':
				RIRCX(pri)->rec_mode |= RECM_TEXT;
				break;
			case  'f':
			case  'F':
				RIRCX(pri)->rec_mode |= RECM_FORMULA;
				break;
		}
	}
#else /* !USE_FRM_WORD */
	RIRCX(pri)->rec_mode = RECM_TEXT;
#endif  /* !USE_FRM_WORD */

	if (ini_fileGET(inif, Enabled_writing_ways, 'S', buffer, PARA * 8) != 0)
	{
		HWRStrCpy((_STR) buffer, (_STR) def_eww);
	}

	RIRCX(pri)->enabled_ww = 0;
	for (i = 0; buffer[i] != 0; i++)
	{
		switch (buffer[i])
		{
			case 'B':
			case 'b':
				RIRCX(pri)->enabled_ww |= WW_BLOCK;
				break;

			case 'P':
			case 'p':
				RIRCX(pri)->enabled_ww |= WW_PALMER;
				break;

			case 'G':
			case 'g':
				RIRCX(pri)->enabled_ww |= WW_GENERAL;
				break;

			case 'R':
			case 'r':
				RIRCX(pri)->enabled_ww |= WW_RESTRICTED;
				break;
		}
	}

	if (ini_fileGET(inif, Enabled_languages, 'S', buffer, PARA * 8) != 0)
	{
		HWRStrCpy((_STR) buffer, (_STR) def_languages);
	}

	RIRCX(pri)->enabled_languages = 0;
	for (i = 0; buffer[i] != 0; i++)
	{
		switch (buffer[i])
		{
			case 'E':
			case 'e':
				RIRCX(pri)->enabled_languages |= EL_ENGLISH;
				break;

			case 'G':
			case 'g':
				RIRCX(pri)->enabled_languages |= EL_GERMAN;
				break;

			case 'F':
			case 'f':
				RIRCX(pri)->enabled_languages |= EL_FRENCH;
				break;

			case 'S':
			case 's':
				RIRCX(pri)->enabled_languages |= EL_SWEDISH;
				break;
		}
	}


	if (ini_fileGET(inif, Enabled_charset, 'S', buffer, PARA * 8) != 0)
	{
		HWRStrCpy((_STR) buffer, (_STR) defcharset);
	}
	/* set enabled charsets */
	RIRCX(pri)->enabled_cs = 0;
	for (i = 0; buffer[i] != 0; i++)
	{
		switch (buffer[i])
		{
			case 'A':
			case 'a':
				RIRCX(pri)->enabled_cs |= CS_ALPHA;
				break;
			case 'N':
			case 'n':
				RIRCX(pri)->enabled_cs |= CS_NUMBER;
				break;
			case 'M':
			case 'm':
				RIRCX(pri)->enabled_cs |= CS_MATH;
				break;
			case 'L':
			case 'l':
				RIRCX(pri)->enabled_cs |= CS_LPUNCT;
				break;
			case 'E':
			case 'e':
				RIRCX(pri)->enabled_cs |= CS_EPUNCT;
				break;
			case 'O':
			case 'o':
				RIRCX(pri)->enabled_cs |= CS_OTHER;
				break;
		}
	}

	if (ini_fileGET(inif, Word_algorithm, 'S',
	                buffer, PARA * 8) != 0)
	{
		HWRStrCpy((_STR) buffer, (_STR) _XrWS);
	}
	/* set word alg */
	RIRCX(pri)->algorithm = 0;
	if (HWRStrCmp((_STR) buffer, (_STR) _XrWS) == 0)
	{
		RIRCX(pri)->algorithm = XRWALG_XRWS;
	}
	if (HWRStrCmp((_STR) buffer, (_STR) _XrSpl) == 0)
	{
		RIRCX(pri)->algorithm = XRWALG_XR_SPL;
	}
	if (HWRStrCmp((_STR) buffer, (_STR) _XrSL) == 0)
	{
		RIRCX(pri)->algorithm = XRWALG_XRSL;
	}
	if (HWRStrCmp((_STR) buffer, (_STR) _XrLWS) == 0)
	{
		RIRCX(pri)->algorithm = XRWALG_XRLWS;
	}
	if (HWRStrCmp((_STR) buffer, (_STR) _XrLW) == 0)
	{
		RIRCX(pri)->algorithm = XRWALG_XRLW;
	}
	if (HWRStrCmp((_STR) buffer, (_STR) _XrExp) == 0)
	{
		RIRCX(pri)->algorithm = XRWALG_EXP;
	}
	if (ini_fileGET(inif, XrWord_mode, 'S',
	                buffer, PARA * 8) != 0)
	{
		HWRStrCpy((_STR) buffer, (_STR) defxrwsmode);
	}
	/* set xrws modes */
	RIRCX(pri)->xrw_mode = 0;
	for (i = 0; buffer[i] != 0; i++)
	{
		switch (buffer[i])
		{
			case 'V':
			case 'v':
				RIRCX(pri)->xrw_mode |= XRWM_VOC;
				break;
			case 'L':
			case 'l':
				RIRCX(pri)->xrw_mode |= XRWM_LD;
				break;
			case 'C':
			case 'c':
				RIRCX(pri)->xrw_mode |= XRWM_CS;
				break;
			case 'T':
			case 't':
				RIRCX(pri)->xrw_mode |= XRWM_TRIAD;
				break;
			case 'B':
			case 'b':
				RIRCX(pri)->xrw_mode |= XRWM_BLOCK;
				break;
			case 'M':
			case 'm':
				RIRCX(pri)->xrw_mode |= XRWM_MWORD;
				break;
			case 'N':
			case 'n':
				RIRCX(pri)->xrw_mode |= XRWM_NONUMFILTER;
				break;
			case 'P':
			case 'p':
				RIRCX(pri)->xrw_mode |= XRWM_NOSSPENL;
				break;
		}
	}

	if (ini_fileGET(inif, Compare_mode, 'S', buffer, PARA * 8) != 0)
	{
		HWRStrCpy((_STR) buffer, (_STR)"");
	}

	RIRCX(pri)->corr_mode = 0;
	for (i = 0; buffer[i] != 0; i++)
	{
		switch (buffer[i])
		{
			case 'C':
			case 'c':
				RIRCX(pri)->corr_mode |= XRCM_CACHE;
				break;
			case 'S':
			case 's':
				RIRCX(pri)->corr_mode |= XRCM_SEPLET;
				break;
		}
	}

	if (ini_fileGET(inif, Xrw_min_wlen, 'I', &(RIRCX(pri)->xrw_min_wlen), 0) != 0)
	{
		RIRCX(pri)->xrw_min_wlen = 0;
	}
	if (ini_fileGET(inif, Xrw_max_wlen, 'I', &(RIRCX(pri)->xrw_max_wlen), 0) != 0)
	{
		RIRCX(pri)->xrw_max_wlen = 0;
	}
#if USE_FRM_WORD
	if (RIRCX(pri)->rec_mode & RECM_FORMULA)
	{
		RIRCX(pri)->enabled_cs |= (CS_NUMBER|CS_MATH);
		RIRCX(pri)->xrw_mode = XRWM_CS;
	}
#endif /* USE_FRM_WORD */

	/* prepare charsets */
	if (ini_fileGET(inif, _Alpha_no_diacritics, 'S',
	                &(pri->andCS[0]), PARA * 8) != 0)
	{
		HWRStrCpy(&(pri->andCS[0]), (_STR) defandcharset);
	}
	if (ini_fileGET(inif, _Alpha_international, 'S',
	                &(pri->aintCS[0]), PARA * 8) != 0)
	{
		HWRStrCpy(&(pri->andCS[0]), (_STR) defaintcharset);
	}
	if (ini_fileGET(inif, _Alpha_charset, 'S',
	                &(pri->aCS[0]), PARA * 8) != 0)
	{
		HWRStrCpy(&(pri->aCS[0]), (_STR) defacharset);
	}
	if (ini_fileGET(inif, _Number_charset, 'S',
	                &(pri->nCS[0]), PARA * 8) != 0)
	{
		HWRStrCpy(&(pri->nCS[0]), (_STR) defncharset);
	}
	if (ini_fileGET(inif, _Math_charset, 'S',
	                &(pri->mCS[0]), PARA * 8) != 0)
	{
		HWRStrCpy(&pri->mCS[0], (_STR) defmcharset);
	}
	if (ini_fileGET(inif, _Leading_puctuation, 'S',
	                &(pri->lCS[0]), PARA * 8) != 0)
	{
		HWRStrCpy(&(pri->lCS[0]), (_STR) deflcharset);
	}
	if (ini_fileGET(inif, _Ending_puctuation, 'S',
	                &(pri->eCS[0]), PARA * 8) != 0)
	{
		HWRStrCpy(&(pri->eCS[0]), (_STR) defecharset);
	}
	if (ini_fileGET(inif, _Other_symbols, 'S',
	                &(pri->oCS[0]), PARA * 8) != 0)
	{
		HWRStrCpy(&(pri->oCS[0]), (_STR) defocharset);
	}

	RIRCX(pri)->alpha_charset = (p_UCHAR) &(pri->aCS[0]);
	RIRCX(pri)->num_charset = (p_UCHAR) &(pri->nCS[0]);
	RIRCX(pri)->math_charset = (p_UCHAR) &(pri->mCS[0]);
	RIRCX(pri)->lpunct_charset = (p_UCHAR) &(pri->lCS[0]);
	RIRCX(pri)->epunct_charset = (p_UCHAR) &(pri->eCS[0]);
	RIRCX(pri)->other_charset = (p_UCHAR) &(pri->oCS[0]);

	prig = pri;
	/* load data and postprocessing scripts */
	if (ini_fileGET(inif, DTE_file_name, 'S',
	                &(pri->dtifname[0]), PARA * 8) != 0)
	{
		pri->dtifname[0] = 0;
#if PG_DEBUG
		printw("Failed to get DTI fname\n");
#endif /* PG_DEBUG */
		return 0;
	}

	if (ini_fileGET(inif, Triads_name, 'S',
	                &(pri->tr_fname[0]), PARA * 8) != 0)
	{
		pri->tr_fname[0] = 0;
#if PG_DEBUG
		printw("Failed to get Triads name.\n");
#endif /* PG_DEBUG */
		return 0;
	}

	//Testing RAM DTE:
	const char* tmpstr = "RAM DTE Name";
	if (ini_fileGET(inif, (p_UCHAR) tmpstr, 'S',
	                szRAMDtiName, 128) != 0)
	{
		szRAMDtiName[0] = 0;
#if PG_DEBUG
		printw("No RAM DTI name.\n");
#endif /* PG_DEBUG */
	}
	//Testing learning:
	if (LoadDtes(pri) == _FALSE)
	{
#if PG_DEBUG
		printw("Failed to load DTI: %s\n", pri->dtifname);
#endif /* PG_DEBUG */
		return 0;
	}
#if PG_DEBUG
	else
	{
		printw("Loaded DTI: %s\n", pri->dtifname);
	}

#endif /* PG_DEBUG */
	/* load vocabulary */
	if (ini_fileGET(inif, Vocabulary_name, 'S',
	                &(pri->voc0name[0]), PARA * 8) != 0)
	{
		pri->voc0name[0] = 0;
#if PG_DEBUG
		printw("Failed to get vocname.\n");
#endif /* PG_DEBUG */
		return 0;
	}

	if (ini_fileGET(inif, PrefVoc_name, 'S', &(pri->voc1name[0]), PARA * 8) != 0)
	{
		pri->voc1name[0] = 0;
#if PG_DEBUG
		printw("No prefix voc name in PS.INI.\n");
#endif /* PG_DEBUG */
	}

	if (ini_fileGET(inif, SuffVoc_name, 'S', &(pri->voc2name[0]), PARA * 8) != 0)
	{
		pri->voc2name[0] = 0;
#if PG_DEBUG
		printw("No suffix voc name in PS.INI.\n");
#endif /* PG_DEBUG */
	}

	if (ini_fileGET(inif, UserVoc_name, 'S', &(pri->voc3name[0]), PARA * 8) != 0)
	{
		pri->voc3name[0] = 0;
#if PG_DEBUG
		printw("No user voc name in PS.INI.\n");
#endif /* PG_DEBUG */
	}


#if USE_ORTOGRAPH
	RIRCX(pri)->fOrto =     0;
	RIRCX(pri)->pDB   = _NULL;

	if ( ini_fileGET(inif, OrtographFlag, 'I', &(RIRCX(pri)->fOrto), 0 ) !=0 )
	{
		printw("\n%s entry not found...",OrtographFlag);
	}
	if (RIRCX(pri)->fOrto)
	{
		if (ini_fileGET(inif,OrtographBase,'S',szLearnDataBase,sizeof(szLearnDataBase)-1)!=0)
		{
			RIRCX(pri)->fOrto = 0;
			printw("\n%s entry not found...",OrtographBase);
			return 0;
		}
	}
	if (RIRCX(pri)->fOrto)
	{
		if(RIRCX(pri)->fOrto & kORTO_EMPTY)
		{
			if ( (RIRCX(pri)->pDB=CreateDataBase(ORTO_BASE_SIZE)) != _NULL )
			{
				printw("\nOrtograph Initialized Successfully, Create [%s]", szLearnDataBase );
			}
			else
			{
				printw("\nERROR : LoadDataBase() Failed !");
				RIRCX(pri)->fOrto = 0;
				return 0;
			}
		}
		else
		{
			if ( (RIRCX(pri)->pDB=LoadDataBase ((p_UCHAR)szLearnDataBase,_NULL)) != _NULL )
			{
				printw("\nOrtograph Initialized Successfully, Load   [%s]", szLearnDataBase );
			}
			else
			{
				printw("\nERROR : LoadDataBase() Failed !");
				RIRCX(pri)->fOrto = 0;
				return 0;
			}
		}
	}
	if (!RIRCX(pri)->fOrto)
	{
		printw("\nOrtograph = OFF");
	}
#endif /* USE_ORTOGRAPH */

#if USE_REJECT
	if ( ini_fileGET(inif, Reject, 'I', &(RIRCX(pri)->fReject), 0 ) !=0 )
	{
		printw("\nReject entry not found...");
	}
	printw("\nReject = %s",(RIRCX(pri)->fReject?"ON":"OFF"));
#endif /* USE_REJECT */

	/* load lex data base */
	if (ini_fileGET(inif, LexDict_name, 'S',
	                &(pri->ldbfname[0]), PARA * 8) != 0)
	{
		pri->ldbfname[0] = 0;
#if PG_DEBUG
		printw("Failed to get LexDict name.\n");
#endif /* PG_DEBUG */
	}

	if (LoadVocs(pri) == _FALSE)
	{
#if PG_DEBUG
		printw("Failed to load some lexicals: %s, %s, %s\n", pri->voc0name, pri->tr_fname, pri->ldbfname);
#endif /* PG_DEBUG */
		return 0;
	}


	/* setting up stupid global flags */
	if (ini_fileGET(inif, Bedit_border, 'I',
	                &pri->fl_bedit_border, 0) != 0)
	{
		pri->fl_bedit_border = 0;
	}
	if (ini_fileGET(inif, num_border, 'I',
	                &pri->fl_numborder, 0) != 0)
	{
		pri->fl_numborder = 0;
	}
	if (ini_fileGET(inif, sep_letter, 'I',
	                &pri->fl_separate_let, 0) != 0)
	{
		pri->fl_separate_let = 0;
	}
	if (ini_fileGET(inif, small_caps, 'I',
	                &pri->fl_small_caps, 0) != 0)
	{
		pri->fl_small_caps = 0;
	}
	if (ini_fileGET(inif, twordcut, 'C',
	                &pri->fl_cut, 0) != 0)
	{
		pri->fl_cut = 0;
	}
	fl_cut = pri->fl_cut;
	if (ini_fileGET(inif, cut_fly, 'C',
	                &pri->fl_cut_fly, 0) != 0)
	{
		pri->fl_cut_fly = 0;
	}

	if (ini_fileGET(inif, Wordcut_Tune, 'C',
	                &(RIRCX(pri)->ws_handle), 0) != 0)
	{
		RIRCX(pri)->ws_handle = 6;
	}

	/* set up garbage flags and baraxlo modes */
	if (ini_fileGET(inif, Sure_level, 'I',
	                &(RIRCX(pri)->sure_level), 0) != 0)
	{
		RIRCX(pri)->sure_level = 85;
	}
	if (ini_fileGET(inif, Answer_level, 'I',
	                &(RIRCX(pri)->answer_level), 0) != 0)
	{
		RIRCX(pri)->answer_level = 15;
	}
	if (ini_fileGET(inif, Answer_allow, 'I',
	                &(RIRCX(pri)->answer_allow), 0) != 0)
	{
		RIRCX(pri)->answer_allow = 30;
	}
	if (ini_fileGET(inif, Xcorr_amnesty, 'I',
	                &(RIRCX(pri)->bad_amnesty), 0) != 0)
	{
		RIRCX(pri)->bad_amnesty = 5;
	}
	if (ini_fileGET(inif, Caps_mode, 'I',
	                &(RIRCX(pri)->caps_mode), 0) != 0)
	{
		RIRCX(pri)->caps_mode = 55;
	}
	if (ini_fileGET(inif, Use_length_limits, 'I',
	                &(RIRCX(pri)->use_len_limits), 0) != 0)
	{
		RIRCX(pri)->use_len_limits = 0;
	}
	if (ini_fileGET(inif, XrWord_tag_size, 'I',
	                &(RIRCX(pri)->xrw_tag_size), 0) != 0)
	{
		RIRCX(pri)->xrw_tag_size = 16;
	}
	if (ini_fileGET(inif, Bad_distance, 'I',
	                &(RIRCX(pri)->bad_distance), 0) != 0)
	{
		RIRCX(pri)->bad_distance = 45;
	}
	if (ini_fileGET(inif, Fly_learn, 'I',
	                &(RIRCX(pri)->fly_learn), 0) != 0)
	{
		RIRCX(pri)->fly_learn = 0;
	}
	if (ini_fileGET(inif, Use_vars_info, 'I',
	                &(RIRCX(pri)->use_vars_inf), 0) != 0)
	{
		RIRCX(pri)->use_vars_inf = 1;
	}
	if (ini_fileGET(inif, Lrn_class_level, 'I',
	                &(RIRCX(pri)->lrn_class_level), 0) != 0)
	{
		RIRCX(pri)->lrn_class_level = 70;
	}
	if (ini_fileGET(inif, Min_class_size, 'I',
	                &(RIRCX(pri)->lrn_min_class), 0) != 0)
	{
		RIRCX(pri)->lrn_min_class = 3;
	}
	if (ini_fileGET(inif, Learn_suff_vars, 'I',
	                &(RIRCX(pri)->lrn_learn_suff), 0) != 0)
	{
		RIRCX(pri)->lrn_learn_suff = 2;
	}
	if (ini_fileGET(inif, Voc_var_reward, 'I',
	                &(RIRCX(pri)->lrn_vocvar_rew), 0) != 0)
	{
		RIRCX(pri)->lrn_vocvar_rew = 5;
	}


	//#if USE_CHUNK_PROCESSOR
	if (ini_fileGET(inif, Use_chunk_processor, 'I',
	                &(RIRCX(pri)->fl_chunk), 0) != 0)
	{
		RIRCX(pri)->fl_chunk = 0;
	}
	if (ini_fileGET(inif, Chunk_letters, 'I',
	                &(RIRCX(pri)->fl_chunk_let), 0) != 0)
	{
		RIRCX(pri)->fl_chunk_let = 0;
	}
	//#endif /* USE_CHUNK_PROCESSOR */

	/* set up postprocessing modes */
#if USE_POSTPROC
#if POSTCALC_FILE_RESULTS
	if (ini_fileGET (inif, PPR_file_name, 'S',
	                 &(PRSFileName[0]), PARA*8) != 0)
	{
		HWRStrCpy(&(PRSFileName[0]), (_STR)none);
	}
	if (ini_fileGET (inif, PPR_min_weight, 'I',
	                 &(RIRCX(pri)->postCtrl.nMinWeight), 0) != 0)
	{
		RIRCX(pri)->postCtrl.nMinWeight = 60;
	}
	if (ini_fileGET (inif, PPR_max_distance, 'I',
	                 &(RIRCX(pri)->postCtrl.nMaxDist), 0) != 0)
	{
		RIRCX(pri)->postCtrl.nMaxDist = 10;
	}
#endif /* POSTCALC_FILE_RESULTS */
	if (ini_fileGET (inif, post, 'C',
	                 &(RIRCX(pri)->fl_post), 0) != 0)
	{
		RIRCX(pri)->fl_post = 1;
	}
#endif /* USE_POSTPROC */

	/* set low-level modes based on the other modes */
	RIRCX(pri)->low_mode = 0;
	if (pri->fl_bedit_border)
	{
		RIRCX(pri)->low_mode |= LMOD_BOX_EDIT;
	}
	else
	{
		RIRCX(pri)->low_mode |= LMOD_FREE_TEXT;
		if ((RIRCX(pri)->enabled_cs & (CS_LPUNCT | CS_EPUNCT)) ||
		        (RIRCX(pri)->rec_mode & RECM_FORMULA) ||
		        (RIRCX(pri)->xrw_mode & XRWM_LD) ||
		        pri->fl_bedit_border
		   )
		{
			RIRCX(pri)->low_mode |= LMOD_CHECK_PUNCTN;
		}
		if (pri->fl_separate_let)
		{
			RIRCX(pri)->low_mode |= LMOD_SEPARATE_LET;
		}
	}
	if ((RIRCX(pri)->rec_mode & RECM_FORMULA) || (pri->fl_numborder != 0))
	{
		RIRCX(pri)->low_mode |= LMOD_BORDER_NUMBER;
	}
	else
	{
		RIRCX(pri)->low_mode |= LMOD_BORDER_GENERAL;
	}


	if (pri->fl_small_caps)
	{
		RIRCX(pri)->low_mode |= LMOD_SMALL_CAPS;
	}

	if (ini_fileGET(inif, multiply_des, 'I',
	                &pri->multiply_des, 0) != 0)
	{
		pri->multiply_des = 0;
	}

	//#ifndef _WIN32        //CHE: GIT< please leave this commented !!!
	HWRMemCpy(&rcm, RIRCX(pri), sizeof(rc_type));
	//#endif /* _WIN32 */   //CHE: GIT< please leave this commented !!!
	if (inif != _NULL)
	{
		HWRTextClose(inif);
	}

	RIRCX(pri)->pFuncYield = RecYield;

	return 1;
} /* end of InitRecInfo */

/* *************************************************************** */
/* *  SetFlagsForRestricted                                      * */
/* *************************************************************** */
_VOID SetFlagsForRestricted(rec_info_type _PTR pri)
{
	if (RIRCX(pri)->enabled_ww == WW_RESTRICTED)
	{
		RIRCX(pri)->low_mode |= LMOD_SEPARATE_LET;
		RIRCX(pri)->corr_mode |= (XRCM_CACHE | XRCM_SEPLET);
		RIRCX(pri)->xrw_mode |= (XRWM_NONUMFILTER | XRWM_NOSSPENL);
		RIRCX(pri)->caps_mode = XCM_DEFAULT |
		                        XCM_FL_DEFSIZE |
		                        XCM_FL_TRYCAPS |
		                        XCM_AL_DEFSIZE |
		                        XCM_AL_TRYCAPS |
		                        XCM_AL_TRYS4C |
		                        XCM_AL_RETC4AV;
	}
} /* end of SetFlagsForRestricted */

/* *************************************************************** */
/* *  LoadDtes                                                   * */
/* *************************************************************** */
_INT LoadDtes(rec_info_type _PTR pri)
{
	_VOID _PTR pdte;
	_VOID _PTR ptr;

	ptr = GetDB(pri, PDB_);
	if (ptr == _NULL ||
	        HWRStrCmp(&(pri->dtifname[0]), \
	                  ((p_dti_descr_type) RIRCX(pri)->dtiptr)->dti_fname) != 0)
	{
		pdte = LoadDB(PDB_, &(pri->dtifname[0]));
		if (pdte)
		{
			ptr = EnableDB((p_VOID) pri, pdte, PDB_);
		}
		if (ptr)
		{
			ptr = FreeDB(ptr, PDB_);
		}
	}
	return (_BOOL) (
	           RIRCX(pri)->dtiptr != NULL);
}

/* *************************************************************** */
/* *   LoadVocs                                                  * */
/* *************************************************************** */
_INT LoadVocs(rec_info_type _PTR pri)
{
	_VOID _PTR pvoc;

	_VOID _PTR ptr;

	ptr = GetDB(pri, TDB_);
	if (ptr == _NULL || HWRStrCmp(&(pri->tr_fname[0]), \
	                              ((p_tr_descr_type) RIRCX(pri)->tr_ptr)->tr_fname) != 0)
	{
		pvoc = LoadDB(TDB_, &(pri->tr_fname[0]));
		if (pvoc)
		{
			ptr = EnableDB((p_VOID) pri, pvoc, TDB_);
#if PG_DEBUG
			printw("Loaded TRD: %s\n", pri->tr_fname);
#endif /* PG_DEBUG */
		}
#if PG_DEBUG
		else
		{
			printw("Failed to load Triads: %s\n", pri->tr_fname);
		}
#endif /* PG_DEBUG */
		if (ptr)
		{
			ptr = FreeDB(ptr, TDB_);
		}
	}

	ptr = GetDB(pri, DDB_);
	if (ptr == _NULL || HWRStrCmp(&(pri->voc0name[0]), \
	                              (_STR) ((vocptr_type _PTR)RIRCX(pri)->vocptr[0])->vocname) != 0)
	{
		pvoc = LoadDB(DDB_, &(pri->voc0name[0]));
		if (pvoc)
		{
			ptr = EnableDB((p_VOID) pri, pvoc, DDB_);
#if PG_DEBUG
			printw("Loaded VOC: %s\n", pri->voc0name);
#endif /* PG_DEBUG */
		}
#if PG_DEBUG
		else
		{
			printw("Failed to load VOC: %s\n", pri->voc0name);
		}
#endif /* PG_DEBUG */
		if (ptr)
		{
			ptr = FreeDB(ptr, DDB_);
		}

		if (RIRCX(pri)->vocptr[1])
		{
			FreeDB(RIRCX(pri)->vocptr[1], DDB_);
		}
		RIRCX(pri)->vocptr[1] = _NULL;
		if (pri->voc1name[0] != 0)
		{
			if ((RIRCX(pri)->vocptr[1] = LoadDB(DDB_, &(pri->voc1name[0]))) == _NULL)
			{
				err_msg("!Failed to load prefix VOC!");
			}
		}

		if (RIRCX(pri)->vocptr[2])
		{
			FreeDB(RIRCX(pri)->vocptr[2], DDB_);
		}
		RIRCX(pri)->vocptr[2] = _NULL;
		if (pri->voc2name[0] != 0)
		{
			if ((RIRCX(pri)->vocptr[2] = LoadDB(DDB_, &(pri->voc2name[0]))) == _NULL)
			{
				err_msg("!Failed to load suffix VOC!");
			}
		}

		if (RIRCX(pri)->vocptr[3])
		{
			FreeDB(RIRCX(pri)->vocptr[3], DDB_);
		}
		RIRCX(pri)->vocptr[3] = _NULL;
		if (pri->voc3name[0] != 0)
		{
			if ((RIRCX(pri)->vocptr[3] = LoadDB(DDB_, &(pri->voc3name[0]))) == _NULL)
			{
				err_msg("!Failed to load user VOC!");
			}
		}
	}

	pvoc = LoadDB(LDB_, &(pri->ldbfname[0]));
	if (pvoc)
	{
		ptr = EnableDB((p_VOID) pri, pvoc, LDB_);
#if PG_DEBUG
		printw("Loaded LDB: %s\n", pri->ldbfname);
#endif /* PG_DEBUG */
	}
#if PG_DEBUG
	else
	{
		printw("Failed to load LDB: %s\n", pri->ldbfname);
	}
#endif /* PG_DEBUG */
	if (ptr)
	{
		ptr = FreeDB(ptr, LDB_);
	}

	return (_BOOL) (RIRCX(pri)->vocptr[0] != _NULL &&
	                RIRCX(pri)->cflptr != _NULL &&
	                RIRCX(pri)->tr_ptr != _NULL);
}

/* *************************************************************** */
/* *     LoadDB                                                  * */
/* *************************************************************** */
p_VOID DLLEXPORT LoadDB(int nDBType, p_CHAR pDBfilename)
{
	p_VOID pdb;
	if (pDBfilename == _NULL)
	{
		return _NULL;
	}

	switch (nDBType)
	{
		case PDB_:
			if (dti_load(pDBfilename, DTI_DTE_REQUEST /*| DTI_PDF_REQUEST*/, &pdb) == 0)
			{
				return pdb;
			}
			else
			{
				break;
			}
		case TDB_:
			if (triads_load(pDBfilename,
			                TR_REQUEST, &pdb) == 0)
			{
				return pdb;
			}
			else
			{
				break;
			}
		case DDB_:
			if (voc_load((p_UCHAR) pDBfilename, &pdb) == 0)
			{
				return pdb;
			}
			else
			{
				break;
			}
		case LDB_:
			if (LdbLoad((p_UCHAR) pDBfilename, &pdb))
			{
				return pdb;
			}
			else
			{
				break;
			}
		default:
			break;
	}
	return _NULL;
}
/* *************************************************************** */
/* *   FreeDB                                                    * */
/* *************************************************************** */
p_VOID DLLEXPORT FreeDB(p_VOID pDB, int nDBType)
{

	switch (nDBType)
	{
		case PDB_:
			if (pDB != _NULL)
			{
				dti_unload(&pDB);
			}
			return pDB;
		case TDB_:
			if (pDB != _NULL)
			{
				triads_unload(&pDB);
			}
			return pDB;
		case DDB_:
			voc_unload(&pDB);
			return pDB;

		case LDB_:
			LdbUnload(&pDB);
			return pDB;

		default:
			break;
	}
	return _NULL;
}

/* *************************************************************** */
/* *     EnableDB                                                * */
/* *************************************************************** */
p_VOID DLLEXPORT EnableDB(p_VOID pRCB, p_VOID pDB, int nDBType)
{
	rec_info_type _PTR pri = (rec_info_type _PTR)pRCB;
	p_VOID pOldPDB;

	if (pRCB == _NULL || pDB == _NULL)
	{
		return _NULL;
	}

	pOldPDB = GetDB(pRCB, nDBType);

	switch (nDBType)
	{
		case PDB_:
			if (pDB == _NULL)
			{
				return pDB;
			}
			RIRCX(pri)->dtiptr = pDB;
			break;
		case TDB_:
			if (pDB == _NULL)
			{
				return pDB;
			}
			RIRCX(pri)->tr_ptr = pDB;
			break;
		case DDB_:
			if (pDB == _NULL)
			{
				return pDB;
			}
			RIRCX(pri)->vocptr[0] = pDB;
			break;

		case LDB_:
			if (pDB == _NULL)
			{
				return pDB;
			}
			RIRCX(pri)->cflptr = pDB;
			break;

		default:
			return _NULL;
	}
	return pOldPDB;
}

/* *************************************************************** */
/* *    GetDB                                                    * */
/* *************************************************************** */
p_VOID DLLEXPORT GetDB(p_VOID pRCB, int nDBType)
{
	rec_info_type _PTR pri = (rec_info_type _PTR)pRCB;

	if (pRCB == _NULL)
	{
		return _NULL;
	}

	switch (nDBType)
	{
		case PDB_:
			return RIRCX(pri)->dtiptr;
		case TDB_:
			return RIRCX(pri)->tr_ptr;
		case DDB_:
			return RIRCX(pri)->vocptr[0];

		case LDB_:
			return RIRCX(pri)->cflptr;

		default:
			break;
	}
	return _NULL;
}

/* ************************************************************************* */
/* *  Set flag for searching this word in tester                           * */
/* ************************************************************************* */

int SetTesterMark(void)
{
	extern p_rec_info_type prig;

#if PG_DEBUG
	((DES_DATA_TYPE _PTR)prig->pmbuf)->custom_flags |= DES_CUSTOM_MARK;
#endif
	return 0;
}

/* *************************************************************** */
/* *  Recognition yield, called from internal loo[s of recognizer  */
/* *************************************************************** */

_INT  RecYield(p_VOID)
{

	return 0;      // Normal Yield end
}

/* *************************************************************** */
/* *     Ini file support routine                                * */
/* *************************************************************** */
/* Gets parameters from ps.ini file                               */
/* Parameters:                                                     */
/*  varname:                                                       */
/*    name of desired variable in ps.ini file. Can be with spaces  */
/*  type:   'S'/'F'/'L'/'I'/'C' - type of information to get/put   */
/*          String/Float(double)/Long/Int/Char                     */
/*  receive_buf: adress, at which to place info of desired type    */
/*  buf_len:                                                       */
/*    for GET string type - max length of string buffer            */
/*    for PUT of double floate - num of digits to save (up to 30)  */
/* Returns:                                                        */
/*    0 - if everything is OK                                      */
/*    Nonzero - error (number of errorous line of ps.ini file).    */
/* *************************************************************** */

_SHORT ini_fileGET(_HFILE fini,
                   p_UCHAR varname,
                   _UCHAR type,
                   p_VOID receive_buf,
                   _SHORT buf_len)
{
	_UCHAR   buf1[MAX_LINELEN];
	_UCHAR   buf2[MAX_LINELEN];

	if (fini == _NULL)
	{
		return 1;
	}
	HWRTextSeek(fini, HWR_TEXT_SEEK_BEGIN);
	/* Step by lines of ini file */
	while (HWRTextGetS((_STR) buf1, MAX_LINELEN - 1, fini) != _NULL)
	{
		*buf2 = 0;
		parse_ini_string(buf1, buf2, _NULL, _NULL);
		if (HWRStrCmp((_STR) varname, (_STR) buf2) == 0)
		{
			*buf2 = 0;
			parse_ini_string(buf1, _NULL, buf2, _NULL);
			switch (type)
			{
				case 'S':
					HWRStrnCpy((_STR) receive_buf, (_STR) buf2, buf_len);
					break;
				case 'C':
					*((p_UCHAR) receive_buf) = (_UCHAR) HWRAtoi((_STR) buf2);
					break;
				case 'I':
					*((p_SHORT) receive_buf) = (_SHORT) HWRAtoi((_STR) buf2);
					break;
				case 'L':
#ifdef _FLAT32
					*((p_LONG) receive_buf) = HWRAtoi((_STR) buf2);
#else /* _FLAT32 */
					*((p_LONG)receive_buf) = HWRAtol((_STR)buf2);
#endif /* _FLAT32 */
					break;
				//        case 'F':
				//         *((double*)receive_buf) = atof(buf2);
				//          break;
				default:
					break;
			}
			return 0;
		}
	}
	return 1;
}

/* *************************************************************** */
/* * parse ini string                                            * */
/* *************************************************************** */
_INT parse_ini_string(_UCHAR _PTR string,
                      _UCHAR _PTR keyword,
                      _UCHAR _PTR value,
                      _UCHAR _PTR comment)
{
	_UCHAR _PTR s;
	_UCHAR _PTR d;
	_BOOL separator;
	_BOOL note;
	_BOOL whitespace;
	_UCHAR buffer[MAX_LINELEN];

	if (string == _NULL)
	{
		goto err;
	}

	s = string;
	d = &buffer[0];
	*d = 0;
	separator = _FALSE;
	note = _FALSE;
	whitespace = _TRUE;
	if (keyword != _NULL)
	{
		*keyword = 0;
	}
	if (value != _NULL)
	{
		*value = 0;
	}
	if (comment != _NULL)
	{
		*comment = 0;
	}
	while (_TRUE)
	{
		switch (*s)
		{
			case ' ': /* space */
			case '\t': /* tab */
				if (whitespace == _FALSE)
				{
					whitespace = _TRUE;
					*d++ = ' ';
					break;
				}
				else
					/* skip all spaces */
				{
					break;
				}
			case ':': /* keyword/value separator */
				if (separator == _FALSE &&
				        whitespace == _TRUE &&
				        note == _FALSE)
				{
					if (d != &buffer[0])
					{
						*d-- = 0;
					}
					/* remove trailing space */
					while (*d == ' ' && d != &buffer[0])
					{
						*d-- = 0;
					}
					if (keyword != _NULL)
					{
						HWRStrCpy((_STR) keyword, (_STR) buffer);
					}
					d = &buffer[0];
					*d = 0;
					separator = _TRUE;
				}
				else
				{
					*d++ = *s;
				}
				break;
			case ';': /* comment delimiter */
				if (note == _FALSE && whitespace == _TRUE)
				{
					if (d != &buffer[0])
					{
						*d-- = 0;
					}
					/* remove trailing space */
					while (*d == ' ' && d != &buffer[0])
					{
						*d-- = 0;
					}
					if (value != _NULL)
					{
						HWRStrCpy((_STR) value, (_STR) buffer);
					}
					d = &buffer[0];
					*d = 0;
					note = _TRUE;
				}
				else
					if (separator == _FALSE)
					{
						note = _TRUE;
					}
					else
					{
						*d++ = *s;
					}
				break;
			case '\0':
			case '\n':
			case '\r':
				if (d != &buffer[0])
				{
					*d-- = 0;
				}
				while (*d == ' ' && d != &buffer[0])
				{
					*d-- = 0;
				}
				if (note == _TRUE)
				{
					if (comment != _NULL)
					{
						HWRStrCpy((_STR) comment, (_STR) buffer);
					}
				}
				else
					if (separator == _TRUE)
					{
						if (value != _NULL)
						{
							HWRStrCpy((_STR) value, (_STR) buffer);
						}
					}
					else
					{
						if (keyword != _NULL)
						{
							*keyword = 0;
						}
						if (value != _NULL)
						{
							*value = 0;
						}
						if (comment != _NULL)
						{
							*comment = 0;
						}
					}
				return 1;
			default: /* all other symbols */
				whitespace = _FALSE;
				*d++ = *s;
				break;
		}
		s++;
	}

err:
	return 0;
}

/* *************************************************************** */
/* *************************************************************** */
/* *************************************************************** */

_BOOL FindIniFile(p_UCHAR buffer, _SHORT bufsize)
{
	_UCHAR _PTR p;
	if (buffer == _NULL)
	{
		return _FALSE;
	}
	GetModuleFileName(NULL, (LPSTR) buffer, bufsize);
	p = buffer + HWRStrLen((_STR) buffer);
	while (p != buffer && *p != '\\')
	{
		p--;
	}
	if (*p == '\\')
	{
		p++;
	}
	HWRStrCpy((_STR) p, "ps.ini");
	return _TRUE;
}

/* *************************************************************** */
/* *************************************************************** */
/* *************************************************************** */

_ULONG TimeTicks(_VOID)
{
	return (_ULONG) GetTickCount();
}

/* *************************************************************** */
/* *************************************************************** */
/* *************************************************************** */
_INT RecognizeNoGroup(rec_info_type _PTR pri)
{

	pri->done_to_point = 1;
	pri->all_data_sent = 1;
	pri->p_word_strokes = _NULL;
	//  pri->num_word       = 0;

	do
	{
		pri->num_points_inrec = (_SHORT) (pri->num_points_got - pri->done_to_point + 1);
		RIRCX(pri)->ii = pri->num_points_inrec;

		if (0 && RIRCX(pri)->algorithm == XRWALG_EXP) //  shake trace for learning XRWALG_XRLWS ) //
		{
			_INT   i, n;
			_INT   ii, base_x, r, dx;
			_TRACE ltr;
			_INT   num_points_got, done_to_point;
			_TRACE pri_trace;

			g_write_des = 0;

			num_points_got = pri->num_points_got;
			done_to_point = pri->done_to_point;
			pri_trace = RITRACE(pri);
			ii = RIRCX(pri)->ii;

			if ((ltr = (_TRACE) HWRMemoryAlloc(sizeof(PS_point_type)*(RIRCX(pri)->ii + 10))) != _NULL)
			{
				HWRMemCpy(ltr, RITRACE(pri) + pri->done_to_point - 1, sizeof(PS_point_type)*(RIRCX(pri)->ii + 5));

				pri->num_points_got = RIRCX(pri)->ii;
				pri->done_to_point = 1;
				pri->pTrace = ltr;

				for (i = 0, base_x = 32000; i < ii; i++)
				{
					if (ltr[i].y < 0)
					{
						continue;
					}
					if (base_x > ltr[i].x)
					{
						base_x = ltr[i].x;
					}
				}

				srand((unsigned) TimeTicks());

				for (n = 0; n < 1; n++)
				{
					r = rand();

					for (i = 0; i < ii; i++)
					{
						if (ltr[i].y < 0)
						{
							continue;
						}
						dx = ltr[i].x - base_x;
						ltr[i].y += (_SHORT) (((r*dx) / ((_INT) RAND_MAX) - dx / 2) / 10);
					}

					RecSubTrace(pri);
				}

				HWRMemoryFree(ltr);
				pri->num_points_got = (_SHORT) num_points_got;
				pri->done_to_point = (_SHORT) done_to_point;
				VRITRACE(pri) = pri_trace;
			}
		}

		g_write_des = 1;

		if (RIRCX(pri)->fl_chunk)
		{
			PegRecWord(pri);
		}
		else
		{
			RecSubTrace(pri);
		}

		pri->done_to_point += (_SHORT) (pri->num_points_inrec - 1);
	}
	while (pri->done_to_point < pri->num_points_got);

	return SUCCESS;
}

/* *************************************************************** */
/* *  EmulateWritingAndRecognize    ****************************** */
/* *************************************************************** */
_INT EmulateWritingAndRecognize(rec_info_type _PTR pri)
{
	if (RIRCX(pri)->fl_chunk)
	{
		return PegRecBunch(pri);
	}
	else
	{
		return SegmAndRecognize(pri);
	}
}

/* *************************************************************** */
/* *  EmulateWritingAndRecognize    ****************************** */
/* *************************************************************** */
_INT SegmAndRecognize(rec_info_type _PTR pri)
{
	_INT          i;
	_INT          num_strokes, len;
	_INT          num_points = pri->num_points_got;
	_INT          err = 0;
	_INT          w_fl, flags;
	PS_point_type _PTR p_tr;
	PS_point_type _PTR stroke;
	word_strokes_type(_PTR w_str)[WS_MAX_WORDS];
	ws_results_type wsr = { 0 };

	PS_point_type _PTR pri_trace = RITRACE(pri);
	extern ws_control_type wsc; // = {0};
	p_ink_info_type ink_info;
	_UCHAR cmp[WS_MAX_WORDS] = { 0 };

#define WST_DEF_H_LINE       80
#define WST_DEF_S_DELAY       3
#define WST_DEF_UNSURE_LEVEL 50

	p_tr = (PS_point_type _PTR)HWRMemoryAlloc(sizeof(PS_point_type)*(num_points + 10));
	w_str = (word_strokes_type(_PTR)[WS_MAX_WORDS])HWRMemoryAlloc(sizeof(*w_str));

	wsr.pwsa = w_str;

	cmp[0] = 0;

#if PG_DEBUG
	DebugRequest(DBG_STROKES, (_ULONG) (&cmp[0]));
#endif /* PG_DEBUG */

	num_strokes = CreateInkInfo((p_VOID) pri_trace, num_points, (p_VOID _PTR)&ink_info);

	if (p_tr == _NULL || num_strokes == 0 || w_str == _NULL)
	{
		err_msg("SegmAndRecognize() -> Failed to create ink_info storage!");
		goto err;
	}

	VRITRACE(pri) = p_tr;
	pri->done_to_point = 1;
	pri->num_word = 0; // Global flag of word number in file
	pri->p_word_strokes = (p_VOID) (&wsr);
	pri->p_strokes_info = (p_VOID) (ink_info);
	RIRCX(pri)->p_ws_wi = (p_VOID) &wswi;

	wsc.num_points = 0;
	wsc.flags = 0;
	wsc.sure_level = RIRCX(pri)->lrn_class_level; // WST_DEF_UNSURE_LEVEL;
	wsc.word_dist_in = RIRCX(pri)->ws_handle; //+1;
	wsc.line_dist_in = 0;
	wsc.def_h_line = WST_DEF_H_LINE;
	wsc.x_delay = (pri->fl_cut_fly) ? WST_DEF_S_DELAY : 0; // Delay in 'letters' or 0 -- only on line end

	wsc.cmp = &cmp;

	w_fl = RM_COMBINE_CARRY;
	flags = 0;
	for (i = 0; i <= num_strokes; i++)
	{
		if (i < num_strokes)
		{
			stroke = (p_PS_point_type) GetInkStrokePtr(i, ink_info);
			len = GetInkStrokeLen(i, ink_info);

			if (stroke == _NULL || len == 0)
			{
				goto err;
			}

			wsc.flags = flags;
			wsc.num_points = len;
			err += WordStrokes(stroke, &wsc, &wsr);
			if (err)
			{
				break;
			}
		}
		else
		{
			wsc.flags |= WS_FL_LAST;
			wsc.num_points = 0;
			err += WordStrokes(_NULL, &wsc, &wsr);
			if (err)
			{
				break;
			}
		}

		if (pri->fl_cut || /*i >= num_strokes-1 */(wsc.flags & WS_FL_LAST))
		{
			while ((len = GetNextWordInkCopy(w_fl, &wsr, (p_VOID) p_tr, ink_info, &wswi)) > 0)
			{
				pri->num_points_inrec = (_SHORT) len;
				RIRCX(pri)->ii = (_SHORT) len;

				err += RecSubTrace(pri);      // Call recognizer

				HWRMemSet(&wswi, 0, sizeof(wswi));
				pri->num_word++;
			}
		}
	}

	VRITRACE(pri) = pri_trace;
	if (w_str)
	{
		HWRMemoryFree(w_str);
	}
	if (p_tr)
	{
		HWRMemoryFree(p_tr);
	}
	DestroyInkInfo((p_VOID _PTR)&ink_info);

	return SUCCESS;

err:
	VRITRACE(pri) = pri_trace;
	if (w_str)
	{
		HWRMemoryFree(w_str);
	}
	if (p_tr)
	{
		HWRMemoryFree(p_tr);
	}
	DestroyInkInfo((p_VOID _PTR)&ink_info);

	return UNSUCCESS;
}


/* *************************************************************** */
/* *  Call PegDll functions to recognize word ******************** */
/* *************************************************************** */
_INT PegRecWord(rec_info_type _PTR pri)
{
	_INT   i, j, num_answ, xrsum;
	_INT   npoints, err;
	p_CGR_point_type strokes;
	static CGRCTX context = _NULL;
	static p_CGR_control_type ctrl;
	_ULONG time, word_time;
	p_UCHAR ptr;
	rec_w_type(_PTR recw)[NUM_RW] = (rec_w_type(_PTR)[NUM_RW])RIRECWORD(pri);

	if (pri->num_word == 0 || context == _NULL)
	{

		if (context)
		{
			CgrCloseContext(context);
		}

		ctrl = PegInitControlData(pri);
		if (!ctrl)
		{
			goto err;
		}

		ctrl->flags = PEG_RECFL_NSEG;

		ctrl->sp_vs_q = 0; //8;

		context = CgrCreateContext();
		if (!context)
		{
			goto err;
		}
	}

	if (CgrOpenSession(ctrl, context))
	{
		goto err;
	}

	npoints = pri->num_points_inrec - 1;
	strokes = (p_CGR_point_type) ((_TRACE) pri->pTrace + pri->done_to_point);

	if (CgrRecognize(npoints, strokes, context))
	{
		goto err;
	}

	time = TimeTicks();

	err = CgrCloseSession(context);

	word_time = TimeTicks() - time;

	num_answ = CgrGetAnswers(CGA_NUM_ALTS, 0, 0, context);
	HWRMemSet(recw, 0, sizeof(*recw));
	for (i = 0; i < num_answ; i++)
	{
		ptr = (p_UCHAR) (CgrGetAnswers(CGA_ALT_WORD, 0, i, context));

		for (j = 0; j < w_lim && ptr[j]; j++)
		{
			(*recw)[i].word[j] = ptr[j];
			(*recw)[i].nvar[j] = 0;
			(*recw)[i].linp[j] = 1;
		}

		(*recw)[i].word[j] = 0;
		(*recw)[i].weight = (_SHORT) (CgrGetAnswers(CGA_ALT_WEIGHT, 0, i, context));
		(*recw)[i].src_id = 1;
		(*recw)[i].attr = 0;
	}

	xrsum = CgrGetAnswers(CGA_ALT_WEIGHT, 0, num_answ, context);
	HWRMemSet(&(*(RIXRDATA(pri))->xrd)[0], 0, sizeof(xrd_el_type)*((RIXRDATA(pri)->size)));
	for (i = 0; i < (RIXRDATA(pri))->size && xrsum; i++)
	{
		(*(RIXRDATA(pri))->xrd)[i].xr.type = 1;
		xrsum--;
		if (xrsum < 255)
		{
			(*(RIXRDATA(pri))->xrd)[i].xr.attrib = (_UCHAR) xrsum;
			xrsum = 0;
		}
		else
		{
			(*(RIXRDATA(pri))->xrd)[i].xr.attrib = 255;
			xrsum -= 255;
		}
	}
	(RIXRDATA(pri))->len = i;
	DebugRequest(DBG_WORDBEG, 0L);
	DbgFillTraceData(RITRACE(pri) + pri->done_to_point - 1, pri->num_points_inrec);
	DbgFillXRData(RIXRDATA(pri));
	DbgFillRWData(recw);

	if (g_write_des)
	{
		((DES_DATA_TYPE _PTR)pri->pmbuf)->time_low = (_USHORT) (1);
		((DES_DATA_TYPE _PTR)pri->pmbuf)->time_upper = (_USHORT) (word_time);
		pri->pctx = _NULL;
		FillDesBuffers(pri);
	}

	pri->num_word++;

	return err;
err:
	return 1;
}

/* *************************************************************** */

_INT PegRecYield(p_VOID);
_INT gflag_more_data_available;
#define PRB_MAX_STROKES 1024

/* *************************************************************** */
/* *  Call PegDll functions to recognize sentences *************** */
/* *************************************************************** */

_INT PegRecBunch(rec_info_type _PTR pri)
{
	_INT  i, j, k;
	_INT  num_ans, num_alts, num_strokes, xrsum;
	_INT  npoints, np;
	_INT  min_sid, max_sid;
	p_INT strokes_num_ptr;
	p_CGR_point_type strokes;
	p_PS_point_type  trace = _NULL;
	static  CGRCTX context = _NULL;
	_ULONG  time, word_time;
	p_CGR_control_type ctrl;
	p_UCHAR ptr;
	rec_w_type(_PTR recw)[NUM_RW] = (rec_w_type(_PTR)[NUM_RW])RIRECWORD(pri);
	_SHORT stroke_info[PRB_MAX_STROKES];


	npoints = pri->num_points_got - 1;
	strokes = (p_CGR_point_type) ((_TRACE) pri->pTrace + 1);

	ctrl = PegInitControlData(pri);
	if (!ctrl)
	{
		goto err;
	}

	ctrl->flags |= PEG_RECFL_TTSEG;
	ctrl->InfoCallBack = PegRecYield;

	if (!context)
	{
		context = CgrCreateContext();
	}
	if (!context)
	{
		goto err;
	}

	if (CgrOpenSession(ctrl, context))
	{
		goto err;
	}

	time = TimeTicks();

	stroke_info[0] = 0;
	gflag_more_data_available = 1;
	for (i = j = 0, k = 1; i < npoints; i++)
	{
		if (strokes[i].y < 0)
		{
			if (CgrRecognize(i - j, &(strokes[j]), context))
			{
				goto err;
			}
			j = i + 1;
			stroke_info[k++] = (_SHORT) j;
			if (k > PRB_MAX_STROKES - 4)
			{
				goto err;    // Just some big enough limit ...
			}
		}
	}


	gflag_more_data_available = 0;

	if (CgrCloseSession(context))
	{
		goto err;
	}

	word_time = TimeTicks() - time;

	trace = (p_PS_point_type) HWRMemoryAlloc(sizeof(PS_point_type)*(npoints + 1));
	if (!trace)
	{
		goto err;
	}
	trace->x = 0;
	trace->y = -1;

	VRITRACE(pri) = trace;
	pri->done_to_point = 1;
	pri->num_word = 0; // Global flag of word number in file

	num_ans = CgrGetAnswers(CGA_NUM_ANSWERS, 0, 0, context);
	for (k = 0; k < num_ans; k++)
	{
		num_alts = CgrGetAnswers(CGA_NUM_ALTS, k, 0, context);
		HWRMemSet(recw, 0, sizeof(*recw));
		for (i = 0; i < num_alts; i++)
		{
			ptr = (p_UCHAR) (CgrGetAnswers(CGA_ALT_WORD, k, i, context));

			for (j = 0; j < w_lim && ptr[j]; j++)
			{
				(*recw)[i].word[j] = ptr[j];
				(*recw)[i].nvar[j] = 0;
				(*recw)[i].linp[j] = 1;
			}

			(*recw)[i].word[j] = 0;
			(*recw)[i].weight = (_SHORT) (CgrGetAnswers(CGA_ALT_WEIGHT, k, i, context));
			(*recw)[i].src_id = 1;
			(*recw)[i].attr = 0;
		}

		xrsum = CgrGetAnswers(CGA_ALT_WEIGHT, k, num_alts, context);
		HWRMemSet(&(*(RIXRDATA(pri))->xrd)[0], 0, sizeof(xrd_el_type)*((RIXRDATA(pri)->size)));
		for (i = 0; i < (RIXRDATA(pri))->size && xrsum; i++)
		{
			(*(RIXRDATA(pri))->xrd)[i].xr.type = 1;
			xrsum--;
			if (xrsum < 255)
			{
				(*(RIXRDATA(pri))->xrd)[i].xr.attrib = (_UCHAR) xrsum;
				xrsum = 0;
			}
			else
			{
				(*(RIXRDATA(pri))->xrd)[i].xr.attrib = 255;
				xrsum -= 255;
			}
		}
		(RIXRDATA(pri))->len = i;
		HWRMemSet(&(*(RIXRDATA(pri))->xrd)[i].xr, 0, sizeof(xrinp_type));

		num_strokes = CgrGetAnswers(CGA_ALT_NSTR, k, 0, context);
		strokes_num_ptr = (p_INT) CgrGetAnswers(CGA_ALT_STROKES, k, 0, context);

		for (i = 0, min_sid = max_sid = strokes_num_ptr[0]; i < num_strokes; i++)
		{
			if (min_sid > strokes_num_ptr[i])
			{
				min_sid = strokes_num_ptr[i];
			}
			if (max_sid < strokes_num_ptr[i])
			{
				max_sid = strokes_num_ptr[i];
			}
		}

		trace->x = (_SHORT) (min_sid);

		for (i = 0, np = 1; i < num_strokes; i++, strokes_num_ptr++)
		{
			j = stroke_info[(*strokes_num_ptr) + 1] - stroke_info[*strokes_num_ptr];
			HWRMemCpy(trace + np, strokes + stroke_info[*strokes_num_ptr], j*sizeof(PS_point_type));
			np += j;
		}

		pri->num_points_inrec = (_SHORT) np;
		trace[np - 1].x = (_SHORT) (max_sid - min_sid + 1);

		DebugRequest(DBG_WORDBEG, 0L);
		DebugRequest(DBG_PASSBEGIN, 0L);
		DbgFillTraceData(trace, (_SHORT) pri->num_points_inrec);
		DbgFillXRData(RIXRDATA(pri));
		DbgFillRWData(recw);
		DebugRequest(DBG_PASSEND, 0L);

		if (g_write_des)
		{
			((DES_DATA_TYPE _PTR)pri->pmbuf)->time_low = (_USHORT) (1);
			((DES_DATA_TYPE _PTR)pri->pmbuf)->time_upper = (_USHORT) (word_time / num_ans);
			pri->pctx = _NULL;
			FillDesBuffers(pri);
		}

		pri->num_word++;
	}


	//  if (context) CgrCloseContext(context);
	if (trace)
	{
		HWRMemoryFree(trace);
	}
	VRITRACE(pri) = strokes - 1;

	return 0;
err:
	if (context)
	{
		CgrCloseContext(context);
	}
	if (trace)
	{
		HWRMemoryFree(trace);
	}
	VRITRACE(pri) = strokes - 1;
	return 1;
}
/* *************************************************************** */
/* *         Init control structure                              * */
/* *************************************************************** */
p_CGR_control_type PegInitControlData(rec_info_type _PTR pri)
{
	static CGR_control_type ctrl = { 0 };
	static int init = 0;

	if (init == 0)
	{
		FILE * file;
		char name[128];

		HWRStrCpy((_STR) name, (_STR) pri->voc0name);
#if VER_PALK_DICT
		HWRStrCat((_STR)name, ".vcb");
#else
		HWRStrCat((_STR) name, ".voc");
#endif

		if ((file = fopen(name, "rb")) != 0)
		{
			if (ElkLoadDict((p_UCHAR) name, (p_VOID _PTR)&ctrl.h_main_dict) == 0)
			{
				init = 1;
			}
			fclose(file);
		}

		if (!init)
		{
			err_msg("!Peg Voc Init Failed!\n");
		}
	}

	return (init == 1) ? &ctrl : 0;
}

/* ************************************************************************** */
/* *  Temp Yield function                                                   * */
/* ************************************************************************** */

_INT PegRecYield(p_VOID)
{

	if (gflag_more_data_available)
	{
		if (rand() < RAND_MAX / 4)
		{
			gflag_more_data_available = 2;
		}
		else
		{
			gflag_more_data_available = 1;
		}
	}

	if (gflag_more_data_available > 1)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

