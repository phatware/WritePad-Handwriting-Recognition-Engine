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

#define STRICT
#define _REQ_WIN
#include <windows.h>
#include <windowsx.h>
#include <memory.h>
#include <stdlib.h>
#include <limits.h>
#ifdef _PENWIN
#include <penwin.h>
#include <hwr_sys.h>
#include <ams_mg.h>
#include <xrw_algs.h>
#include <xr_names.h>
#include <xrword.h>
#include <learn.h>
#else
#include "pensub.h32"
#include "ams_mg.h"
#endif
#include <bastypes.h>
#include <wg_stuff.h>
#include <xr_names.h>
#include <grlib.h>
#include "cdcmain.h"
#include "wggbl.h"
#include "wgidm.h"
#include "wgmdi.h"
#include "wgxrd.h"
#include "wgtrc.h"
#include "wgrec.h"
#include "wgdbo.h"
#include "wgink.h"
#include "wgtap.h"
#include "wgprf.h"
#include "wgxed.h"
#include "wgmsg.h"
#include "wgtls.h"
#include "wgsta.h"
#include "wgsrl.h"
#include "wgdbg.h"
#include "wgdde.h"
#include "wghlv.h"
#include "WGTemplate.h"
#include "wginteractivelrn.h"
#include "resource.h"

#define GSHND (GMEM_MOVEABLE | GMEM_ZEROINIT | GMEM_SHARE)

#define INSERTLAST         1
#define OVERWRITE          3


#define MAX_DLG_ANSWERS   5
#define MIN_DLG_ANSWERS   1
#define DLG_OFFSET        2

#define HLV_MAX_SECTS     32

#define EMPTY_SECTION      0
#define JUSTMADE_SECTION   1
#define SEMIFILLED_SECTION 2
#define FILLED_SECTION     3

#define MAX_VARIANT        10
#define MAX_PARTS          20
#define LEFT_MARGIN        10
#define TOP_MARGIN         2

typedef HPEN  HPENARRAY[3];

typedef struct _CORANSW_TYPE
{
	int             n_rec_words;
	int             max_length;
	int             word_index;
	int             ilast;
	char            last[LAB_RW_SIZE];
	GUIDE           guide;
	HFONT           hFont;
	//  p_LAB_RECWORD   rec_words;
	LAB_RECWORD_TYPE  rec_words[LAB_NUM_RW];
} CORANSW_TYPE, FAR *p_CORANSW_TYPE;

typedef struct _TRAJ_TYPE
{
	int             np_trace;
	POINT           trace;
} TRAJ_TYPE, FAR *p_TRAJ_TYPE;

typedef struct _HLV_SECT_TYPE
{
	WORD            sectionType;
	_SHORT          PassNum;
	int             infoType;
	int             index;
	struct _HLV_SECT_TYPE FAR *next;
	struct _HLV_SECT_TYPE FAR *prev;
	// lo_level data
	int             LineCount;
	p_LINE          lpLineArray;
	int             PointCount;
	p_COLORPOINT    lpPointArray;
	// end of lo_level data
	RECT            DataRect;
	int             np_lo_fl_trace;
	LPPOINT         lo_fl_trace;
	int             np_lo_trace;
	LPPOINT         lo_trace;
	TRACE_TYPE      trace;
	int             nLineCoords;
	LPPOINT         lppUpper;
	LPPOINT         lppLower;
	int             n_xrs;
	int             n_rec_words;
	int             n_answers;
#ifdef _PENWIN
	p_XR            xrs;
	p_rec_w_type    rec_words;
	p_answ_type     answers;
#else
	p_LAB_XRDATA    xrs;
	int             IndexDiffXr;
	p_LAB_RECWORD   rec_words;
	p_ANSWER        answers;
	WORDTRAJ        wordstraj;
#endif
	BOOL            fRcFilled;
	LPRECT          lprcWords;
	int             selword;
	GRAPH_TYPE      graph;
	int             chw, chh;
#if DEBUG_CHUNK_PROCESSOR
	p_LAB_CHUNK_DATA lpChunk;
	p_LAB_LETTERPOS  lpLetterPos;
#endif
	int             selcor;
	int             nCompr;
	LPPOINT         lpCompressed;
	int             nDecompr;
	LPPOINT         lpDecompressed;

	int             range_begin;
	int             range_end;
	LPRECT          pxrrect;
	LPRECT          pxrrect_input;
	p_PPD           ppd;
} HLV_SECT_TYPE, FAR * LP_HLV_SECT;

typedef struct _HLV_TYPE
{
	BOOL fValid;
	BOOL fUpperDone;
	RECT rcInk;
	RECT rcXrs;
	RECT rcAnswer;
	RECT rcGraph;
	RECT rcPAnswer;
	int  nSects;
	LP_HLV_SECT   lpCurSect;
	LP_HLV_SECT   head;
	LP_HLV_SECT   tail;
	char          szTAPFileName[_MAX_PATH];
} HLV_TYPE, FAR * LP_HLV;

typedef struct
{
	int          version;
	int          xrnumber;
	p_LAB_XRDATA pxr_input;
	p_LAB_XRDATA pxr_ext;
} XRINPUTSHOW_TYPE;
TRACEDATA_TYPE traceData = { 0, NULL };

TRACEDATA_TYPE lineData = { 0, NULL };
BOOL fAutoEnlarge = FALSE;
/* This variable defines whether coming result goes into newly
   created section (if gCurSect < 0) or in old one */
static int  gCurSect = -1;
static int  gFirstWordNum = 0;

#ifdef _PENWIN
static BOOL bGetXRData   = TRUE;
#endif

/* This flag is set after successful call to EditXRLine dialog box.
   When it is set we copy new XR line (lpNewXrLine) into recognizer XR_DATA array
   and skip "low_level" subroutine.
   Flag is cleared immidiatly after copying lpNewXrLine into recognizer data */
static BOOL         bGetNewXrData = FALSE;
static LP_HLV_SECT  lpEditSection = NULL;
#ifdef _PENWIN
static p_XR         lpNewXrLine = NULL;
#else
static p_LAB_XRDATA lpNewXrLine = NULL;
#endif


/* This variables define where to insert newly created section */
int         gInsertBranchSection = 0;
LP_HLV_SECT lpTAPSect;

/* section type */
int         SectionType = BOUNDARY;


#ifdef _PENWIN
RC       rcx;
XR4UPPER XrU;
#endif

static void ShowXRCorrelation(LP_HLV_SECT lpSect, int xrnumber, POINT pt);
static void ShowXRInput(LP_HLV_SECT lpSect, int xrnumber, POINT pt);
static void RemoveSection(LP_HLV lpHlv, LP_HLV_SECT lpSect);
static void KillBranchSections(HWND hWnd, int SectNum);
static void UpdateSectNumber(LP_HLV_SECT lpSect);
static void DrawGraph(HDC hDC, LP_HLV_SECT lpSect, LPRECT lprc, HFONT hFont);
COLORREF    GetGraphColor(_UCHAR weight);
static void GetSymbolRect(p_GRAPH_TYPE graph, LPRECT rcWnd,
                          int index, int width, int height, LPRECT rcSym);
static BOOL SelectNewChar(HWND hWnd, LP_HLV_SECT lpSect, LP_HLV lpHlv, LPARAM lParam,
                          int SelectedChar);
static void DrawCharSelRect(HDC hDC, LPRECT rc, int Color);
static void DrawTrace(HDC hDC, LP_GLB_DATA_TYPE glb, LP_HLV_SECT lpSect,
                      BOOL DrawGraph, BOOL DrawCompressionResult);
static void DrawXr(HDC hDC, LP_HLV_SECT lpSect, LPRECT lprc, BOOL DrawGraph);
static p_PPD SetGraphPPDData(LP_HLV_SECT lpSect);
static void DrawSelectedCharWeights(HDC hDC, p_GRAPH_TYPE graph,
                                    LPRECT lprc, int ch_width, int i);
static int  GetSelectedChar(p_GRAPH_TYPE graph, WORD Arrow);
static void DrawSections(HDC hDC, LP_HLV lpHlv, LPRECT lprc);
static void DrawSect(HDC hDC, LP_HLV_SECT lpSect, int *x, int *y, int InitY);
static int  SaveEditedXRLine(LP_HLV lpHlv, LP_HLV_SECT lpSect);
static void ModifyXRLine(LP_HLV_SECT lpSect, short beg, short end);
static int  GetNextGraphSymbol(p_GRAPH_TYPE graph, int j);
static int  GetPrevGraphSymbol(p_GRAPH_TYPE graph, int j);
static int  GetLeftGraphSymbol(p_GRAPH_TYPE graph, int j, int row, int col);
static int  GetRightGraphSymbol(p_GRAPH_TYPE graph, int j, int row, int col);
static void DrawSymbol(HDC hDC, HPEN hPen, LP_GLB_DATA_TYPE glb,
                       LP_HLV_SECT lpSect, LPSTR lpSymb);
static p_PPD SetRWPPDData(LP_HLV_SECT lpSect, int sel_word, DWORD *GraphWord);
static void FillPPDElement(p_PPD Dest, LPSTR pSource,
                           char symb, char rsymb,
                           int variant, int percent);
static BOOL GetGraphWord(p_GRAPH_TYPE graph, short *i, LPSTR buff);
//static void MakeRecWordFromPPD(p_PPD ppd, p_rec_w_type rec_word) ;
static void AddWordToGraph(LPSTR rws, LPSTR Word, UINT *j);
static void AddOneWordToGraph(LPSTR prws, LPSTR Word, UINT *j);
static void FillLowData(HWND hWnd, LP_HLV_SECT lpSect);
static void DrawLowLevelData(HWND hWnd, HDC hDC, LP_GLB_DATA_TYPE glb, LP_HLV_SECT lpSect);
static void DrawLettersOnTrajectory(HDC hDC, LP_GLB_DATA_TYPE glb,
                                    HPENARRAY hPens, LP_HLV_SECT lpSect,
                                    p_LAB_RWGNODE pGraph, BOOL DrawGraph);

#if DEBUG_CHUNK_PROCESSOR
static void DrawChunk(HDC hDC, LP_GLB_DATA_TYPE glb, p_LAB_CHUNK_DATA lpChunk) ;
static void DrawObjects(HDC hDC, LP_GLB_DATA_TYPE glb, LP_HLV_SECT lpSect) ;
static void DrawChunkAnswer(HDC hDC, LP_HLV_SECT lpSect, LPRECT rcAnswer, LPSTR newword) ;
#endif

static BOOL SetWindowsData(HWND hWnd, WORD WindowID, LP_HLV_SECT lpSect,
                           BOOL Reset, BOOL Delete);
static void DrawAnswers(HDC hDC, LPRECT lprc, p_CORANSW_TYPE lpAnswers, BOOL bChangeFont);
static void DrawGuidesAndAnswers(HDC hDC, p_CORANSW_TYPE lpAnswers, LPRECT lprc, int word_number);
static void DrawLetter(HDC hDC, p_CORANSW_TYPE lpAnswers, LPRECT lprc, int line, int letter);
static void DrawResult(HWND hWnd, p_CORANSW_TYPE lpAnswers, char Answer,
                       DWORD line_col, BOOL *bChangeFont);
static void SaveCorrection(HWND hWnd, int index, LPSTR Answer);
static void SetTrajectoryGuide(HWND hWnd, p_CORANSW_TYPE lpAnswers,
                               LPGUIDE guide, DWORD *line_col, RECT BoundingRect);
static void DrawButtons(HDC hDC, LPRECT rc, LONG Id);
static void CompressTrajectory(LP_HLV_SECT lpSect, WPARAM wParam);
static void DrawCompressionResult(HDC hDC, LP_HLV_SECT lpSect, LP_HLV lpHlv);

#ifndef _PENWIN
static int ReadRecognizerData(LP_HLV_SECT lpSect);
static int ReadGraphData(LP_HLV_SECT lpSect, BOOL AfterPP);
static int  AllocNewGraph(p_GRAPH_TYPE pSavedGraph, int size);
static int  FreeGraph(p_GRAPH_TYPE pSavedGraph);
static int  CopyGraph(p_GRAPH_TYPE pSavedGraph, p_GRAPH_TYPE pOldGraph);
static int  CopyGraphData(LPSTR Dest, LPSTR Src, int BeginIndex, int size);
static int  MergeGraphs(p_GRAPH_TYPE pDest, p_GRAPH_TYPE p1, p_GRAPH_TYPE p2, LPSTR Word);
#endif

#ifdef _PENWIN
#define GRSYMBOL(x, i)  (char)(x[i].sym)
#define GRRSYMBOL(x, i) (char)(x[i].rnrsym)
#define GRTYPE(x, i)    x[i].type
#define GRUSER(x, i)    x[i].d_user
#define GRWEIGHT(x, i)  x[i].weight
#define GRNVAR(x, i)    x[i].nvar
#else
#define GRSYMBOL(x, i)  (x[i].rnasym)
#define GRRSYMBOL(x, i) (x[i].rnrsym)
#define GRTYPE(x, i)    x[i].rntype
#define GRUSER(x, i)    x[i].rnuser
#define GRWEIGHT(x, i)  x[i].rnweight
#define GRNVAR(x, i)    x[i].rnvar
#endif
#define GRCOL(x, i)     HIWORD(x[i])
#define GRROW(x, i)     LOWORD(x[i])

extern dbgSetDebugMode_TYPE SetDebugMode;
static HWND hAnswerDlgBox = NULL;
static HWND hCorrectDlgBox = NULL;
static HWND hCorResult = NULL;
/****************************************************************/
static void TrashSect(LP_HLV_SECT lpSect)
{
	trcDone(&(lpSect->trace));
	DISPOSE(lpSect->lppUpper, "WGHLV TrashSect");
	DISPOSE(lpSect->lppLower, "WGHLV TrashSect");
	DISPOSE(lpSect->xrs, "WGHLV TrashSect");
	DISPOSE(lpSect->lprcWords, "WGHLV TrashSect");
	DISPOSE(lpSect->lo_trace, "WGHLV TrashSect");
	DISPOSE(lpSect->answers, "WGHLV TrashSect");
	DISPOSE(lpSect->rec_words, "WGHLV TrashSect");
#if DEBUG_CHUNK_PROCESSOR
	DISPOSE(lpSect->lpChunk, "WGHLV TrashSect");
	DISPOSE(lpSect->lpLetterPos, "WGHLV TrashSect");
#endif
	DISPOSE(lpSect->lpLineArray, "WGHLV TrashSect");
	DISPOSE(lpSect->lpPointArray, "WGHLV TrashSect");
	DISPOSE(lpSect->lpCompressed, "WGHLV TrashSect");
	DISPOSE(lpSect->lpDecompressed, "WGHLV TrashSect");
	DISPOSE(lpSect->ppd, "WGHLV TrashSect");
	DISPOSE(lpSect->pxrrect, "WGHLV TrashSect");
	DISPOSE(lpSect->pxrrect_input, "WGHLV TrashSect");
	FreeGraph(&lpSect->graph);
	lpSect->sectionType = EMPTY_SECTION;
	lpSect->nLineCoords = 0;
	lpSect->n_xrs = 0;
	lpSect->selword = -1;
	lpSect->np_lo_trace = 0;
	lpSect->n_answers = 0;
	lpSect->n_rec_words = 0;
	lpSect->LineCount = 0;
	lpSect->PointCount = 0;
	lpSect->nCompr = 0;
	lpSect->nDecompr = 0;
	lpSect->IndexDiffXr = -1;
} /* TrashSect */

/*******************************************************************/
static void InsertNewSection(LP_HLV lpHlv, LP_HLV_SECT lpNewSection,
                             int nAfterSect, BOOL InsertRightAfterCurSect)
{

	LP_HLV_SECT  lpSect;

	lpNewSection->prev = lpHlv->tail;
	lpNewSection->next = NULL;
	if (InsertRightAfterCurSect)
	{
		lpNewSection->prev = lpHlv->lpCurSect;
		lpNewSection->next = lpHlv->lpCurSect->next;
		if (lpHlv->lpCurSect->next != NULL)
		{
			lpHlv->lpCurSect->next->prev = lpNewSection;
		}
		lpHlv->lpCurSect->next = lpNewSection;
		if (lpNewSection->next == NULL)
		{
			lpHlv->tail = lpNewSection;
		}
		return;
	}
	if (lpHlv->head == NULL)
	{
		lpHlv->head = lpHlv->tail = lpNewSection;
	}
	else
	{
		if (nAfterSect < 0)
		{
			// add new section to the list end
			lpHlv->tail->next = lpNewSection;
			lpHlv->tail = lpNewSection;
		}
		else
		{
			lpSect = lpHlv->head;
			while (lpSect->index != nAfterSect)           // find first section with given index
			{
				lpSect = lpSect->next;
			}
			while (lpSect && lpSect->index == nAfterSect) // find last section with given index
			{
				lpSect = lpSect->next;
			}
			if (lpSect == NULL)                          // last section in the list
			{
				lpHlv->tail->next = lpNewSection;
				lpHlv->tail = lpNewSection;
			}
			else                                         // insert section to the end of the branch
			{
				lpNewSection->prev = lpSect->prev;
				lpNewSection->next = lpSect;
				lpSect->prev->next = lpNewSection;
				lpSect->prev = lpNewSection;
			}
		}
	}
} /* end of InsertNewSection */

/*******************************************************************/
static int BuildSectionList(LP_HLV lpHlv, int nWords)
{
	int         i;
	LP_HLV_SECT p, p1;

	lpHlv->head = (LP_HLV_SECT) DebugAllocPtr(GSHND, sizeof(HLV_SECT_TYPE), "WGHLV BuildSectionList");
	if (lpHlv->head == NULL)
	{
		return FALSE;
	}
	p = lpHlv->head;
	p->sectionType = EMPTY_SECTION;
	p->index = 0;
	p->next = NULL;
	p->prev = NULL;
	for (i = 0; i < nWords - 1; i++)
	{
		p->next = (_HLV_SECT_TYPE*) DebugAllocPtr(GSHND, sizeof(HLV_SECT_TYPE), "WGHLV BuildSectionList");
		if (p->next == NULL)
		{
			p1 = lpHlv->head;
			while (p1 != NULL)
			{
				p = p1;
				DebugFreePtr(p, "WGHLV BuildSectionList");
				p1 = p1->next;
			}
			return FALSE;
		}
		p->next->next = NULL;
		p->next->prev = p;
		p->next->index = i + 1;
		p->next->sectionType = EMPTY_SECTION;
		p = p->next;
	}
	lpHlv->tail = p;
	lpHlv->lpCurSect = lpHlv->head;
	return TRUE;
} /* end of BuildSectionList */

/*******************************************************************/
static LP_HLV_SECT FindSectByNumber(LP_HLV lpHlv, int num)
{
	int         n;
	LP_HLV_SECT lpSect;

	for (n = 0, lpSect = lpHlv->head;
	        lpSect != NULL && num != lpSect->index; n++, lpSect = lpSect->next);
	return lpSect;
} /* end of FindSectByNumber */

/*******************************************************************/
static int FindSectNumber(LP_HLV lpHlv, LP_HLV_SECT lpSect)
{
	int         n;
	LP_HLV_SECT lpSect0;

	for (lpSect0 = lpHlv->head, n = 0;
	        lpSect0 != NULL && lpSect0 != lpSect;
	        lpSect0 = lpSect0->next, n++);
	if (lpSect0 != NULL)
	{
		return lpSect0->index;
	}
	else
	{
		return -1;
	}
} /* end of FindSectNumber */

/*******************************************************************/
static LP_HLV_SECT FindNextWord(LP_HLV lpHlv, BOOL fForward)
{
	LP_HLV_SECT lpSect, lpTail;
	BOOL        ListTAP;

	lpSect = lpHlv->lpCurSect;
	ListTAP = prfListTAPWords();
	if (ListTAP && lpHlv->head->index < 0)
		// no open TAP file jet
	{
		ListTAP = FALSE;
	}
	if (!ListTAP)
	{
		// look through all the sections
		if (fForward)
		{
			lpSect = (lpSect->next == NULL) ? (lpHlv->head) : (lpSect->next);
		}
		else
		{
			lpSect = (lpSect->prev == NULL) ? (lpHlv->tail) : (lpSect->prev);
		}
	}
	else
	{
		// look only through TAP file sections
		lpTail = lpHlv->head;
		while (lpTail->next != NULL && lpTail->next->index >= 0)
		{
			lpTail = lpTail->next;
		}
		if (fForward)
		{
			if (lpSect->index < 0)
			{
				lpSect = lpHlv->head;
			}
			else
			{
				lpSect = (lpSect == lpTail) ? (lpHlv->head) : (lpSect->next);
			}
		}
		else
		{
			if (lpSect->index < 0)
			{
				lpSect = lpTail;
			}
			else
			{
				lpSect = (lpSect->prev == NULL) ? (lpTail) : (lpSect->prev);
			}
		}
	}
	return lpSect;
} /* end of FindNextWord */

/*******************************************************************/
static void TrashAll(LP_HLV lpHlv, BOOL KillAll)
{
	int         nSect;
	LP_HLV_SECT lpSect, lpSectNext, lpNewTail;
	BOOL        SelNewSect;

	lpSect = lpHlv->head;
	if (!KillAll)
	{
		// kill only not TAP sections
		nSect = 0;
		while (lpSect != NULL && lpSect->index >= 0)
		{
			lpSect = lpSect->next;
			nSect++;
		}
		lpNewTail = lpSect == NULL ? lpHlv->tail : lpSect->prev;
	}
	SelNewSect = FALSE;
	while (lpSect != NULL)
	{
		lpSectNext = lpSect->next;
		if (lpHlv->lpCurSect == lpSect)
		{
			SelNewSect = TRUE;
		}
		TrashSect(lpSect);
		DebugFreePtr(lpSect, "WGHLV TrashAll");
		lpSect = lpSectNext;
	}
	if (!KillAll)
	{
		lpHlv->tail = lpNewTail;
		lpHlv->tail->next = NULL;
		lpHlv->nSects = nSect;
		if (SelNewSect == TRUE)
		{
			lpHlv->lpCurSect = lpHlv->tail;
		}
	}
	else
	{
		lpHlv->nSects = 0;
		lpHlv->lpCurSect = NULL;
		lpHlv->head = lpHlv->tail = NULL;
	}
} /* end of TrashAll */

/*******************************************************************/
static void DrawLowLevelData(HWND hWnd, HDC hDC, LP_GLB_DATA_TYPE glb, LP_HLV_SECT lpSect)
{
	int     i;
	for (i = 0; i < lpSect->PointCount; i++)
	{
		inkDrawPixel(hDC, glb, lpSect->lpPointArray[i].x,
		             lpSect->lpPointArray[i].y,
		             lpSect->lpPointArray[i].color, TRUE);
	}
	for (i = 0; i < lpSect->LineCount; i++)
	{
		inkDrawLine(hWnd, NULL, &(lpSect->lpLineArray[i]), glb);
	}
} /* end of DrawLowLevelData */

/*******************************************************************/
static void DrawLine(HDC hDC, LP_GLB_DATA_TYPE glb, LP_HLV_SECT lpSect)
{
	HPEN hPenOld, hPen;
	if (lpSect->nLineCoords <= 0)
	{
		return;
	}
	hPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 255));  /* Magenta */
	hPenOld = (HPEN) SelectObject(hDC, hPen);
	glbPolyline(glb, hDC, lpSect->lppUpper, lpSect->nLineCoords);
	glbPolyline(glb, hDC, lpSect->lppLower, lpSect->nLineCoords);
	SelectObject(hDC, hPenOld);
	if (hPen != NULL)
	{
		DeleteObject(hPen);
	}
} /* end of DrawLine */

/*******************************************************************/
static void DrawLettersOnTrajectory(HDC hDC, LP_GLB_DATA_TYPE glb,
                                    HPENARRAY hPens, LP_HLV_SECT lpSect,
                                    p_LAB_RWGNODE pGraph, BOOL GraphMode)
{
	WORDTRAJECTORY_TYPE   Data;
	int                   i, result, iend, ibeg, j;
	p_GRAPH_TYPE          graph = &lpSect->graph;

	if (lpSect->answers == NULL || pGraph == NULL)
	{
		return;
	}
	Data.ptraj = &lpSect->wordstraj;
	Data.pxr = lpSect->xrs;
	// draw symbol graph
	if (GraphMode)
	{
		for (i = 0; i < 2; i++)
		{
			Data.pgraph = (p_LAB_RWGNODE) &pGraph[i];
			result = msgGetWordTraj((LPSTR) &Data);
			if (result)
			{
				SelectObject(hDC, hPens[i % 3]);
				for (j = 0; j < lpSect->wordstraj.parts_number[0]; j++)
				{
					ibeg = lpSect->wordstraj.parts[j].ibeg;
					iend = lpSect->wordstraj.parts[j].iend;
					glbPolyline(glb, hDC, &lpSect->lo_trace[ibeg], iend - ibeg + 1);
				}
				if (i >= (int) graph->size - 1 || GRTYPE(pGraph, i + 1) != LAB_RWST_SYM ||
				        GRUSER(pGraph, i + 1) != GRUSER(pGraph, i))
					// don't draw second char
				{
					break;
				}
			}
		}
		return;
	}
	// draw word graph
	for (i = 0; lpSect->answers[lpSect->selword].anword[i] != 0; i++)
	{
		// draw i-th letter of answer word
		Data.pgraph = (p_LAB_RWGNODE) &pGraph[i];
		result = msgGetWordTraj((LPSTR) &Data);
		if (result)
		{
			SelectObject(hDC, hPens[i % 3]);
			for (j = 0; j < lpSect->wordstraj.parts_number[0]; j++)
			{
				ibeg = lpSect->wordstraj.parts[j].ibeg;
				iend = lpSect->wordstraj.parts[j].iend;
				glbPolyline(glb, hDC, &lpSect->lo_trace[ibeg], iend - ibeg + 1);
			}
		}
	}

} /* end of DrawLettersOnTrajectory() */

/*******************************************************************/
static void DrawSymbol(HDC hDC, HPEN hPen, LP_GLB_DATA_TYPE glb,
                       LP_HLV_SECT lpSect, LPSTR lpSymb)
{
	int       i, k, j, beg, end, xr_beg, xr_end;
#ifdef _PENWIN
	p_RWG_type lpSymbol = (p_RWG_type)lpSymb;
#else
	p_LAB_RWGNODE lpSymbol = (p_LAB_RWGNODE) lpSymb;
#endif

	SelectObject(hDC, hPen);
	for (i = 0; i < lpSymbol->rnxlen; i++)
	{
		j = i + lpSymbol->rnxbeg;
#ifdef _PENWIN
		if (lpSect->xrs[j].xr <= X_ZN)
			// break xr element
		{
			continue;
		}
		xr_beg = lpSect->xrs[j].begpoint;
		xr_end = lpSect->xrs[j].endpoint;
#else // _PENWIN not defined
		if (lpSect->xrs[j].xdxr.xnxr <= X_ZN)
			// break xr element
		{
			continue;
		}
		xr_beg = lpSect->xrs[j].xdbegpt;
		xr_end = lpSect->xrs[j].xdendpt;
#endif
		k = xr_beg;
		beg = xr_beg;
		end = xr_end;
		while (k <= xr_end)
		{
			if (lpSect->lo_trace[k].y == -1)
			{
				end = k - 1;
				glbPolyline(glb, hDC, &lpSect->lo_trace[beg], end - beg + 1);
				beg = k + 1;
				end = xr_end;
			}
			k++;
		}
		glbPolyline(glb, hDC, &lpSect->lo_trace[beg], end - beg + 1);
	}
} /* end of DrawSymbol */

/*******************************************************************/
#if DEBUG_CHUNK_PROCESSOR
static void DrawChunk(HDC hDC, LP_GLB_DATA_TYPE glb,
                      p_LAB_CHUNK_DATA lpChunk)
{
	int   i, ib, ie;
	HPEN  hPens[2], hOldPen;

	hPens[0] = CreatePen(PS_SOLID, 1, RGB(255, 255, 255)); /* White */
	hPens[1] = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));     /* Green */
	hOldPen = SelectObject(hDC, hPens[0]);
	for(i = 0; i < lpChunk->nChNumb; i++)
	{
		if (lpChunk->pCh[i].wType != AIR_CHUNK)
		{
			SelectObject(hDC, hPens[i%2]);
			ib = lpChunk->pCh[i].iFirst;
			ie = lpChunk->pCh[i].iLast;
			glbMoveTo( glb, hDC, lpChunk->pWApx[ib].x, lpChunk->pWApx[ib].y);
			while(ib < ie)
			{
				ib++;
				glbLineTo(glb, hDC, lpChunk->pWApx[ib].x, lpChunk->pWApx[ib].y);
			}
		}
	}
	SelectObject(hDC, hOldPen);
	DeleteObject(hPens[0]);
	DeleteObject(hPens[1]);
} /* end of DrawChunk */

/*******************************************************************/
static void DrawChunkAnswer(HDC hDC, LP_HLV_SECT lpSect,
                            LPRECT rcAnswer, LPSTR newword)
{
	COLORREF        Color[4];
	p_LAB_LOWOBJ    lpLoObject;
	int             i, length;
	RECT            rc;
	SIZE            size;

	Color[0] = RGB(0, 255, 0);     /* Green */
	Color[1] = RGB(0, 0, 255);     /* Blue  */
	Color[2] = RGB(255, 0, 0);     /* Red   */
	Color[3] = RGB(255, 255, 255); /* White */
	length = lstrlen(newword);
	CopyRect(&rc, rcAnswer);
	lpLoObject = &lpSect->lpChunk->pLO[0];
	for(i = 0; i < length; i++)
	{
		if (lpLoObject[i].wAttr != 0xFFFF)
		{
			SetTextColor(hDC, Color[i%4]);
		}
		else
		{
			SetTextColor(hDC, RGB(128, 128, 128));
		}
		DrawText(hDC, &newword[i], 1, &rc, DT_LEFT | DT_VCENTER);
		GetTextExtentPoint(hDC, &newword[i], 1, &size);
		rc.left += size.cx;
	}
	rc.left += 8*size.cx;
	rc.right = rcAnswer->right;
#ifdef _PENWIN
	DrawText(hDC, lpSect->rec_words->word,
	         lstrlen(lpSect->rec_words->word), &rc, DT_LEFT | DT_VCENTER);
#else
	DrawText(hDC, lpSect->rec_words->rwword,
	         lstrlen(lpSect->rec_words->rwword), &rc, DT_LEFT | DT_VCENTER);
#endif
} /* end of DrawChunkAnswer */

/*******************************************************************/
static void DrawObjects(HDC hDC, LP_GLB_DATA_TYPE glb, LP_HLV_SECT lpSect)
{
	p_LAB_LOWOBJ    lpLoObject;
	int             i, j, s_beg, s_end;
	HPEN            hPens[4], hOldPen;
	LPPOINT         lppPoints = lpSect->lo_trace;

	hPens[0] = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));     /* Green */
	hPens[1] = CreatePen(PS_SOLID, 1, RGB(0, 0, 255));     /* Blue  */
	hPens[2] = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));     /* Red   */
	hPens[3] = CreatePen(PS_SOLID, 1, RGB(255, 255, 255)); /* White */
	hOldPen = SelectObject(hDC, hPens[0]);
	lpLoObject = &lpSect->lpChunk->pLO[0];
	for(i = 0; i < lpSect->lpChunk->nLONumb; i++)
	{
		if (lpLoObject[i].wAttr != 0xFFFF)
		{
			s_beg = lpLoObject[i].iBegTr;
			s_end = lpLoObject[i].iEndTr;
			j = s_beg;
			while( j < s_end)
			{
				if (lppPoints[j].y == -1)
				{
					glbPolyline(glb, hDC, &lppPoints[s_beg], j-1-s_beg);
					s_beg = j+1;
				}
				j++;
			}
			if (lppPoints[s_end].y == -1)
			{
				s_end--;
			}
			glbPolyline(glb, hDC, &lppPoints[s_beg], s_end-s_beg);
			SelectObject(hDC, hPens[(i+1)%4]);
		}
	}
	SelectObject(hDC, hOldPen);
	for(i = 0; i < 4; i++)
	{
		if (hPens[i])
		{
			DeleteObject(hPens[i]);
		}
	}
} /* end of DrawObjects */

#endif    // DEBUG_CHUNK_PROCESSOR
/*******************************************************************/
/* famous and real DrawTrace !!! */
static void DrawTrace(HDC hDC, LP_GLB_DATA_TYPE glb, LP_HLV_SECT lpSect,
                      BOOL DrawGraph, BOOL DrawCompressionResult)
{
	int            s_beg, s_end;
	int            i, k;
	LP_TRACE       lpTrace = &(lpSect->trace);
	LPPOINT        lppPoints = lpTrace->lppPoints;
	HPEN           hPenOld, hPenNone;
	HPENARRAY      hPens;
	p_GRAPH_TYPE   graph = &lpSect->graph;
	DWORD          GraphWord;

	if (DebugLevel <= -300)
	{
		hPenNone = CreatePen(PS_SOLID, 1, RGB(128, 128, 128));    /* Blue  */
	}
	else
	{
		hPens[0] = CreatePen(PS_SOLID, 1, RGB(255, 255, 255)); /* White */
		hPens[1] = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));     /* Green */
		hPens[2] = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));     /* Red   */
		hPenNone = CreatePen(PS_SOLID, 1, RGB(0, 0, 255));     /* Blue  */
	}
	// draw all the trajectory in blue color
	if (DrawCompressionResult)
	{
		hPenOld = (HPEN) SelectObject(hDC, hPens[1]);
	}
	else
	{
		hPenOld = (HPEN) SelectObject(hDC, hPenNone);
	}
	if (lppPoints && lpTrace->lpiStrokes)
	{
		for (i = 0; i < lpTrace->nStrokes; i++)
		{
			s_beg = lpTrace->lpiStrokes[i];
			s_end = lpTrace->lpiStrokes[i + 1];
			glbPolyline(glb, hDC, &lppPoints[s_beg], s_end - s_beg);
		}
	}
	if (DebugLevel <= -300)
	{
		SelectObject(hDC, hPenOld);
		DeleteObject(hPenNone);
		return;
	}
	if (DrawGraph)
	{
		if (prfShowXROnTrajectory())
		{
			// show XR elements of selected symbol(s) on trajectory
			for (i = 0; i < (int) graph->size; i++)
			{
				if (graph->pselb[i])
				{
					DrawSymbol(hDC, hPens[0], glb, lpSect, (LPSTR) &graph->prwsb[i]);
					if (i < (int) graph->size - 1 && GRTYPE(graph->prwsb, i + 1) == LAB_RWST_SYM &&
					        GRUSER(graph->prwsb, i + 1) == GRUSER(graph->prwsb, i))
					{
						DrawSymbol(hDC, hPens[1], glb, lpSect, (LPSTR) &graph->prwsb[i + 1]);
					}
					break;
				}
			}
		}
		else
		{
			// show one ore two letters of selected word  on trajectory
			for (i = 0; i < (int) graph->size; i++)
			{
				if (graph->pselb[i])
				{
					DrawLettersOnTrajectory(hDC, glb, hPens,
					                        lpSect, (p_LAB_RWGNODE) &graph->prwsb[i], DrawGraph);
					break;
				}
			}
		}
	}
	else
	{
		GraphWord = 0;
		SetRWPPDData(lpSect, lpSect->selword, &GraphWord);
		if (GraphWord)
		{
			if (prfShowXROnTrajectory())
			{
				// show XR elements on trajectory
				for (i = 0; i < (int) HIWORD(GraphWord); i++)
				{
					k = LOWORD(GraphWord) + i;
					DrawSymbol(hDC, hPens[i % 3], glb, lpSect, (LPSTR) &graph->prwsb[k]);
				}
			}
			else
			{
				// show letters on trajectory
				k = LOWORD(GraphWord);
				// k == begin index of selected word in graph
				DrawLettersOnTrajectory(hDC, glb, hPens,
				                        lpSect, (p_LAB_RWGNODE) &graph->prwsb[k], DrawGraph);
			}
		}
	}
	SelectObject(hDC, hPenOld);
	for (i = 0; i < 3; i++)
		if (hPens[i] != NULL)
		{
			DeleteObject(hPens[i]);
		}
	if (hPenNone != NULL)
	{
		DeleteObject(hPenNone);
	}
} /* end of real DrawTrace !!! */

/*******************************************************************/
static void DrawRect(HDC hDC, LPRECT lpRect)
{
	MoveToEx(hDC, lpRect->left, lpRect->top, NULL);
	LineTo(hDC, lpRect->right, lpRect->top);
	LineTo(hDC, lpRect->right, lpRect->bottom);
	LineTo(hDC, lpRect->left, lpRect->bottom);
	LineTo(hDC, lpRect->left, lpRect->top);
} /* end of DrawRect */

/*******************************************************************/
static void DrawCompressionResult(HDC hDC, LP_HLV_SECT lpSect, LP_HLV lpHlv)
{
	HPEN      hPen, hPenOld;
	char      buff[128];
	LOGFONT   lf;
	HFONT     hFont, hFontOld;
	long      ratio;
	int       y, size;

	hPen = CreatePen(PS_SOLID, 1, RGB(255, 128, 0));
	hPenOld = (HPEN) SelectObject(hDC, hPen);
	_fmemset(&lf, 0, sizeof(LOGFONT));
	lf.lfHeight = -(lpHlv->rcAnswer.bottom - lpHlv->rcAnswer.top) / 4;
	lstrcpy(lf.lfFaceName, "Courier New");
	hFont = CreateFontIndirect(&lf);
	if (hFont != NULL)
	{
		hFontOld = (HFONT) SelectObject(hDC, hFont);
	}
	MoveToEx(hDC, lpHlv->rcAnswer.left, lpHlv->rcAnswer.top, NULL);
	LineTo(hDC, lpHlv->rcAnswer.right, lpHlv->rcAnswer.top);
	size = lpSect->np_lo_trace*sizeof(POINT);
	SetTextColor(hDC, RGB(0, 255, 0));
	wsprintf(buff, "Original trajectory size     %.4d bytes, %d points.",
	         size, lpSect->np_lo_trace);
	y = lpHlv->rcAnswer.top + TOP_MARGIN;
	TextOut(hDC, lpHlv->rcAnswer.left, y, buff, lstrlen(buff));
	y += -lf.lfHeight;
	ratio = size - lpSect->nCompr;
	ratio = ratio * 100 / size;
	wsprintf(buff, "Compressed trajectory size   %.4d bytes, compression = %d%%",
	         lpSect->nCompr, ratio);
	TextOut(hDC, lpHlv->rcAnswer.left, y, buff, lstrlen(buff));
	y += -lf.lfHeight;
	wsprintf(buff, "Decompressed trajectory size %.4d bytes, %d points.",
	         lpSect->nDecompr*sizeof(POINT), lpSect->nDecompr);
	TextOut(hDC, lpHlv->rcAnswer.left, y, buff, lstrlen(buff));
	if (hFont != NULL)
	{
		SelectObject(hDC, hFontOld);
		DeleteObject(hFont);
	}
	SelectObject(hDC, hPenOld);
	DeleteObject(hPen);
} /* end of DrawCompressionResult */

/*******************************************************************/
static void DrawAnswer(HDC hDC, LP_HLV_SECT lpSect, LP_HLV lpHlv, BOOL ShowGraph)
{
	int         nFontSize, nMaxWords, n, x, y, len, w, h, nXOff, nYOff;
	int         selword = lpSect->selword, weight;
	LPRECT      lprcWords = lpSect->lprcWords;
	LOGFONT     lf;
	HFONT       hFont, hFontOld;
	HPEN        hPen, hPenSel, hPenOld;
	COLORREF    clr;
	_SHORT      flags;
	char        tempword[LAB_RW_SIZE];
	char        buffer[LAB_RW_SIZE], newword[LAB_RW_SIZE];
	RECT        rc;
	SIZE        size;
	int         n_answers = lpSect->n_answers;
	int         n_rec_words = lpSect->n_rec_words;
#ifdef _PENWIN
	p_rec_w_type  rec_words   = lpSect->rec_words;
	p_answ_type answers       = lpSect->answers;
#else
	p_LAB_RECWORD  rec_words = lpSect->rec_words;
	p_ANSWER       answers = lpSect->answers;
#endif

	hPen = CreatePen(PS_SOLID, 1, RGB(255, 128, 0));
	hPenSel = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));

	h = lpHlv->rcAnswer.bottom - lpHlv->rcAnswer.top;
	w = lpHlv->rcAnswer.right - lpHlv->rcAnswer.left;
	hPenOld = (HPEN) SelectObject(hDC, hPen);
	MoveToEx(hDC, lpHlv->rcAnswer.left, lpHlv->rcAnswer.top, NULL);
	LineTo(hDC, lpHlv->rcPAnswer.right, lpHlv->rcPAnswer.top);
	MoveToEx(hDC, lpHlv->rcAnswer.right, lpHlv->rcAnswer.top, NULL);
	LineTo(hDC, lpHlv->rcAnswer.right, lpHlv->rcAnswer.bottom);
	MoveToEx(hDC, lpHlv->rcGraph.right, lpHlv->rcGraph.top, NULL);
	LineTo(hDC, lpHlv->rcGraph.right, lpHlv->rcGraph.bottom);

	nMaxWords = n_answers > n_rec_words ? n_answers : n_rec_words;
	if (ShowGraph)
	{
		nMaxWords = LAB_XR_SIZE + 1;
	}
	nFontSize = (h - 2) / (nMaxWords + 1);

	_fmemset(&lf, 0, sizeof(LOGFONT));
	lf.lfHeight = -nFontSize;
	lstrcpy(lf.lfFaceName, "Courier New");
	hFont = CreateFontIndirect(&lf);
	if (hFont != NULL)
	{
		hFontOld = (HFONT) SelectObject(hDC, hFont);
		nXOff = lpHlv->rcAnswer.left + LEFT_MARGIN;
		nYOff = lpHlv->rcAnswer.top + TOP_MARGIN;
		if (lpSect->answers)
		{
			for (n = 0; n < n_answers; n++)
			{
				flags = answers[n].anflags;
				lstrcpy(tempword, answers[n].anword);
				len = lstrlen(tempword);
				if (len == 0)
				{
					break;
				}
				if (flags & 1)
				{
					clr = RGB(0, 255, 255);
				}
				if (flags & 2)
				{
					clr = RGB(0, 255, 0);
				}
				if ((flags & 1) && (flags & 2))
				{
					clr = RGB(255, 0, 255);
				}
				SetTextColor(hDC, clr);
				SetBkColor(hDC, RGB(0, 0, 0));
				x = nXOff;
				y = nYOff + n*nFontSize;
				len = lstrlen(tempword);
				wchar_t wbuf[40] = { 0, };
				mbstowcs(wbuf, tempword, 40);
				//prfEncode((LPSTR)tempword, newword, TRUE);
				ExtTextOutW(hDC, x, y, 0, NULL, wbuf, len, NULL);
				if (lprcWords != NULL)
				{
					GetTextExtentPoint(hDC, newword, len, &size);
					SetRect(&rc, x, y, x + size.cx, y + size.cy);
					CopyRect(&(lprcWords[n]), &rc);
				}
				wsprintf((LPSTR) buffer, (LPSTR) "%lx %3d %3d.%d %3d ",
				         answers[n].ansources,
				         answers[n].anstat,
				         answers[n].anweight / 10, answers[n].anweight % 10,
				         answers[n].anpercent);
				len = lstrlen(buffer);
				GetTextExtentPoint(hDC, buffer, len, &size);
				x = lpHlv->rcAnswer.right - size.cx;
				ExtTextOut(hDC, x, y, 0, NULL, buffer, len, NULL);
			} /* for n < n_answers */
		}
		nXOff = lpHlv->rcPAnswer.left + LEFT_MARGIN;
		nYOff = lpHlv->rcPAnswer.top + TOP_MARGIN;
		n = 0;
		while (n < n_rec_words)
		{
#ifdef _PENWIN
			lstrcpy(tempword, (LPSTR)rec_words[n].word);
			weight = rec_words[n].weight;
#else
			lstrcpy(tempword, (LPSTR) rec_words[n].rwword);
			weight = rec_words[n].rwweight;
#endif
			len = lstrlen(tempword);
			if (len == 0)
			{
				break;
			}
			clr = RGB(128, 128, 128);
			SetTextColor(hDC, clr);
			SetBkColor(hDC, RGB(0, 0, 0));
			x = nXOff;
			y = nYOff + n*nFontSize;
			prfEncode((LPSTR) tempword, newword, TRUE);
			wchar_t wbuf[40] = { 0, };
			mbstowcs(wbuf, newword, 40);
			ExtTextOutW(hDC, x, y, 0, NULL, wbuf, len, NULL);
			wsprintf((LPSTR) buffer, (LPSTR) "%3d", weight);
			len = lstrlen(buffer);
			GetTextExtentPoint(hDC, buffer, len, &size);
			x = lpHlv->rcPAnswer.right - size.cx;
			ExtTextOut(hDC, x, y, 0, NULL, buffer, len, NULL);
			n++;
		} /* for n < nPPWords */

#if DEBUG_CHUNK_PROCESSOR
		if (lpSect->lpChunk != NULL)
		{
			int i;
			x = nXOff;
			y = nYOff + n*nFontSize;
			lstrcpy(tempword, (LPSTR)lpSect->lpChunk->szAnswer);
			len = lstrlen(tempword);
			for(i = 0; i < len; i++)
			{
				if (tempword[i] == '\xff')
				{
					tempword[i] = '?';
				}
			}
			prfEncode(tempword, newword, TRUE);
			if (lpSect->lpChunk->fIsNumber)
			{
				SetTextColor(hDC, RGB(0,255,0));
				ExtTextOut(hDC, x, y, 0, NULL, newword, len, NULL);
			}
			else
				if (!lpSect->lpChunk->fLetters)
				{
					SetTextColor(hDC, RGB(255,0,0));
					ExtTextOut(hDC, x, y, 0, NULL,
					           "No chunk proccessing",
					           lstrlen("No chunk proccessing"),
					           NULL);
				}
		}

		if (lpSect->lpLetterPos != NULL && lpSect->lpChunk->fLetters)
		{
			int i;
			SIZE size;

			x = nXOff;
			y = nYOff;
#ifdef _PENWIN
			lstrcpy(tempword, (LPSTR)rec_words[0].word);
#else
			lstrcpy(tempword, (LPSTR)rec_words[0].rwword);
#endif
			len = lstrlen(tempword);
			if (len == 0)
			{
				goto NoAnswer;
			}
			SetBkColor(hDC, RGB(0, 0, 0));
			prfEncode((LPSTR)tempword, newword, TRUE);
			for (i = 0; newword[i] != '\0'; i++)
			{
				if (lpSect->lpLetterPos->insLetters[i] != 0)
				{
					SetTextColor(hDC, RGB(0,255,0)); /* Green */
				}
				else
				{
					SetTextColor(hDC, RGB(128,128,128)); /* Grey */
				}
				ExtTextOut(hDC, x, y, 0, NULL, newword+i, 1, NULL);
				GetTextExtentPoint(hDC, newword+i, 1, &size);
				x += (int) size.cx;
			}
NoAnswer:
			x = nXOff;
			y = nYOff + (n+1)*nFontSize;
			SetTextColor(hDC, RGB(128,128,128)); /* Grey */
			lstrcpy(tempword, lpSect->lpLetterPos->oldAnswer);
			len = lstrlen(tempword);
			if (len == 0)
			{
				goto NoOldAnswer;
			}
			SetBkColor(hDC, RGB(0, 0, 0));
			prfEncode((LPSTR)tempword, newword, TRUE);
			ExtTextOut(hDC, x, y, 0, NULL, newword, len, NULL);
NoOldAnswer:
			;
		}

#endif
		if (!ShowGraph)
		{
			if (lprcWords != NULL)
			{
				lpSect->fRcFilled = TRUE;
			}
			else
			{
				lpSect->fRcFilled = FALSE;
			}
			if (selword >= 0 && lpSect->fRcFilled)
			{
				SelectObject(hDC, hPenSel);
				DrawRect(hDC, &(lprcWords[selword]));
			}
		}
		else
		{
			DrawGraph(hDC, lpSect, &lpHlv->rcGraph, hFont);
		}
		SelectObject(hDC, hFontOld);
		DeleteObject(hFont);
	} /* if hFont != NULL */
	SelectObject(hDC, hPenOld);
	if (hPenSel != NULL)
	{
		DeleteObject(hPenSel);
	}
	if (hPen != NULL)
	{
		DeleteObject(hPen);
	}
} /* DrawAnswer */

/*******************************************************************/
static void DrawXr(HDC hDC, LP_HLV_SECT lpSect, LPRECT lprc, BOOL DrawGraph)
{
	int             n, k;
	int             n_xrs = lpSect->n_xrs;
	int             x, y, ixr;
	int             nXOff, nYOff, nFontSize;
	int             prevlinp, delta;
	LOGFONT         lf;
	HFONT           hFont, hFontOld;
	char            EncodeBuf[2] = " ";
	DWORD           wh;
#ifdef _PENWIN
	p_XR          xrs  = lpSect->xrs;
	XR_TYPE       xr;
#else
	p_LAB_XRDATA    xrs = lpSect->xrs;
	LAB_XRDATA_TYPE xr;
#endif

	//CHE: for stripped version:
#ifndef  STRIP_PENLAB
#error  Include header with  STRIP_PENLAB !!!!
#endif
#if  STRIP_PENLAB
	return;
#endif /*STRIP_PENLAB*/
	nFontSize = (lprc->bottom - lprc->top) / 8;
	nXOff = lprc->left + nFontSize / 2;
	nYOff = lprc->top;

#ifdef _PENWIN
	for (n = 0; n < n_xrs && xrs[n].xr != 0; n++)
	{
		xrdDraw(hDC, nXOff + n*nFontSize*2/3, nYOff +nFontSize,
		        -nFontSize, (LPSTR)&(xrs[n]), 1, FALSE, NULL);
	}
#else
	DISPOSE(lpSect->pxrrect_input, "WGHLV DrawXr");
	lpSect->pxrrect_input = (LPRECT) DebugAllocPtr(GSHND, (n_xrs + 1)*sizeof(RECT), "WGHLV FillTrace");
	for (n = 0; n < n_xrs && xrs[n].xdxr.xnxr != 0; n++)
	{
		LAB_XRDATA_TYPE  xrst = xrs[n];
#ifdef VERTIGO
		xrst.xdxr.xnxr += 190-32;
#endif
		wh = xrdDraw(hDC, nXOff + n*nFontSize * 2 / 3, nYOff + nFontSize,
		             -nFontSize, (LPSTR) &(xrs[n]), 1, FALSE,
		             &lpSect->pxrrect_input[n]);
		if (lpSect->IndexDiffXr == n)
		{
			HPEN   hPen, hPenOld;
			int    y = nYOff + nFontSize;
			hPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
			hPenOld = (HPEN) SelectObject(hDC, hPen);
			MoveToEx(hDC, nXOff + n*nFontSize * 2 / 3, y, NULL);
			LineTo(hDC, nXOff + (n + 1)*nFontSize * 2 / 3, y);
			LineTo(hDC, nXOff + (n + 1)*nFontSize * 2 / 3, y + nFontSize);
			LineTo(hDC, nXOff + n*nFontSize * 2 / 3, y + nFontSize);
			LineTo(hDC, nXOff + n*nFontSize * 2 / 3, y);
			SelectObject(hDC, hPenOld);
			DeleteObject(hPen);
			//        lpSect->IndexDiffXr = -1;
		}
	}
#endif
	DISPOSE(lpSect->ppd, "WGHLV DrawXr");
	DISPOSE(lpSect->pxrrect, "WGHLV DrawXr");
	if (!DrawGraph)
	{
		if (lpSect->selword < 0)
		{
			return;
		}
		lpSect->ppd = SetRWPPDData(lpSect, lpSect->selword, NULL);
		if (!lpSect->ppd)
		{
			return;
		}
	}
	else
	{
		lpSect->ppd = SetGraphPPDData(lpSect);
		if (!lpSect->ppd)
		{
			return;
		}
	}
	_fmemset(&lf, 0, sizeof(LOGFONT));
	lf.lfHeight = -nFontSize;
	lstrcpy(lf.lfFaceName, "Courier New");
	hFont = CreateFontIndirect(&lf);
	if (hFont != NULL)
	{
		hFontOld = (HFONT) SelectObject(hDC, hFont);
	}

	//??SD
	ixr = 0;
	for (n = 0; n < LAB_RW_SIZE && lpSect->ppd[n].letter != 0; n++)
	{
		for (k = 0; k < LAB_XR_SIZE && lpSect->ppd[n].xr[k] != 0; k++)
		{
			ixr++;
		}
	}
	lpSect->pxrrect = (LPRECT) DebugAllocPtr(GSHND, (ixr + 1)*sizeof(RECT), "WGHLV FillTrace");
	y = 0;
	prevlinp = -1;
	ixr = 0;
	for (n = 0; n < LAB_RW_SIZE && lpSect->ppd[n].letter != 0; n++)
	{
		delta = (prevlinp == lpSect->ppd[n].alias[0] || prevlinp == -1) ?
		        (lpSect->ppd[n].alias[0] + 1) : (lpSect->ppd[n].alias[0]);
		x = nXOff + delta*nFontSize * 2 / 3;
		//EncodeBuf[0] = lpSect->ppd[n].letter;
		//prfEncode(EncodeBuf, EncodeBuf, TRUE);
		SetTextColor(hDC, RGB(255, 255, 255));
		//ExtTextOut(hDC, x, nYOff, 0, NULL, EncodeBuf, 1, NULL);
		wchar_t wcsbuf[2] = { 0, };
		wcsbuf[0] = lpSect->ppd[n].letter;
		ExtTextOutW(hDC, x, nYOff, 0, NULL, wcsbuf, 1, NULL);

		for (k = 0; k < LAB_XR_SIZE && lpSect->ppd[n].xr[k] != 0; k++)
		{
			x = lpSect->ppd[n].alias[k];
			if (prevlinp == lpSect->ppd[n].alias[k])
			{
				y++;
			}
			else
			{
				y = 0;
			}
#ifdef _PENWIN
			xr.xr = lpSect->ppd[n].xr[k];
			xr.a  = lpSect->ppd[n].attr[k];
			xr.h  = lpSect->ppd[n].height[k];
			xr.p  = 0;
#else
			xr.xdxr.xnxr = lpSect->ppd[n].xr[k];
			xr.xdxr.xna = lpSect->ppd[n].attr[k];
			xr.xdxr.xnh = lpSect->ppd[n].height[k];
			xr.xdxr.xnp = 0;
#endif
			wh = xrdDraw(hDC, nXOff + x*nFontSize * 2 / 3, nYOff + (y + 3)*nFontSize,
			             -nFontSize, (LPSTR) &xr, 0, FALSE, NULL);
			prevlinp = lpSect->ppd[n].alias[k];
			SetRect(&(lpSect->pxrrect[ixr]),
			        nXOff + x*nFontSize * 2 / 3, nYOff + (y + 3)*nFontSize,
			        nXOff + x*nFontSize * 2 / 3 + LOWORD(wh),
			        nYOff + (y + 3)*nFontSize + HIWORD(wh));
			ixr++;
		}
	}
	if (hFont != NULL)
	{
		SelectObject(hDC, hFontOld);
		DeleteObject(hFont);
	}
} /* end of DrawXr */

/*******************************************************************/
static void FillTrace(LP_HLV_SECT lpSect, LPGUIDE lpg, WORD PassNum)
{
	LPPOINT             lppt, lppPoints, lppUpper, lppLower;
	LPINT               lpiStrokes, lpiWords;
	unsigned char FAR   *lpb;
	int                 np, n, nPoints, nStrokes;
	int                 i, j;
	int                 dx, left, top, bottom;
	long                h;
	RECT                rc;

	lpSect->selword = 0; // ??
#ifdef _PENWIN
	if (traceData.TDpt == NULL)
	{
		return;
	}
	lppt = traceData.TDpt;
	np   = traceData.TDnp;
	lpSect->np_lo_trace = traceData.TDnp;
	DISPOSE(lpSect->lo_trace, "WGHLV FillTrace");
	lpSect->lo_trace = (LPPOINT) DebugAllocPtr(GSHND, np*sizeof(POINT), "WGHLV FillTrace");
	_fmemcpy(lpSect->lo_trace, traceData.TDpt, np*sizeof(POINT));
#else
	//??SD lpSect->lo_trace already contains trace, no need for global traceData
	lppt = lpSect->lo_trace;
	np = lpSect->np_lo_trace;
#endif

	if (lppt[np - 1].y < 0)
	{
		np--;    /* Workaround of last y==-1 point */
	}
	nPoints = nStrokes = 0;
	for (n = 0; n < np; n++)
	{
		if (lppt[n].y < 0)
		{
			nStrokes++;
		}
		else
		{
			nPoints++;
		}
	}

	trcDone(&(lpSect->trace));
	if (trcInit(&(lpSect->trace), nPoints, nStrokes, 1))
	{
		lppPoints = lpSect->trace.lppPoints;
		lpiStrokes = lpSect->trace.lpiStrokes;
		lpiWords = lpSect->trace.lpiWords;

		lpiStrokes[0] = 0;
		for (i = 0, j = 0, n = 0; n < np; n++)
		{
			if (lppt[n].y < 0)
			{
				lpiStrokes[i++] = j;
			}
			else
			{
				lppPoints[j].x = lppt[n].x;
				lppPoints[j].y = lppt[n].y;
				j++;
			}
		}
		lpiStrokes[nStrokes] = j;
		lpiWords[0] = 0;
		lpiWords[1] = nStrokes;
		trcCalcMetrics(&(lpSect->trace));
		/* Fill guide field */
		if (lpg != NULL)
		{
			_fmemcpy(&(lpSect->trace.lpgWords[0]), lpg, sizeof(GUIDE));
		}
		/* Fill curved line */
		if (lineData.TDpt != NULL)
		{
			lpb = (unsigned char*) lineData.TDpt;
			np = lineData.TDnp;
			trcMeasureWord(&(lpSect->trace), 0, &rc);
			dx = (rc.right - rc.left) / (np - 1);
			left = rc.left;
			top = rc.top;
			bottom = rc.bottom;
			h = bottom - top;
			DISPOSE(lpSect->lppUpper, "WGHLV FillTrace");
			lppUpper = (LPPOINT) DebugAllocPtr(GSHND, np * sizeof(POINT), "WGHLV FillTrace");
			lpSect->lppUpper = lppUpper;
			lppLower = (LPPOINT) DebugAllocPtr(GSHND, np * sizeof(POINT), "WGHLV FillTrace");
			DISPOSE(lpSect->lppLower, "WGHLV FillTrace");
			lpSect->lppLower = lppLower;
			for (i = 0; i < np; i++)
			{
				lppUpper[i].x = lppLower[i].x = left + i*dx;
				lppLower[i].y = top + (int) (h * (long) lpb[i * 2] / 255L);
				lppUpper[i].y = top + (int) (h * (long) lpb[i * 2 + 1] / 255L);
			}
			lpSect->nLineCoords = np;
		}
	} /* if trcInit */

	DISPOSE(lineData.TDpt, "WGHLV FillTrace");
	if (PassNum == 1)
	{
		// second pass
		DISPOSE(traceData.TDpt, "WGHLV FillTrace");
	}
} /* end of FillTrace */

/*******************************************************************/
static void FillLowData(HWND hWnd, LP_HLV_SECT lpSect)
{
	LPINKDATA       lpData;
	GLOBALHANDLE    hData;

	hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
	lpData = (LPINKDATA) DebugLockHandle(hData);

	lpSect->lpLineArray = (p_LINE) DebugAllocPtr(GSHND, lpData->LineCount*sizeof(LINE_TYPE),
	                      "WGHLV FillLowData");
	if (lpSect->lpLineArray == NULL)
	{
		return;
	}
	lpSect->lpPointArray = (p_COLORPOINT) DebugAllocPtr(GSHND,
	                       lpData->PointCount*sizeof(COLORPOINT_TYPE),
	                       "WGHLV FillLowData");
	if (lpSect->lpPointArray == NULL)
	{
		DISPOSE(lpSect->lpLineArray, "WGHLV FillLowData");
		return;
	}
	lpSect->LineCount = lpData->LineCount;
	lpSect->PointCount = lpData->PointCount;
	_fmemcpy(lpSect->lpLineArray, lpData->LineArray, lpData->LineCount*sizeof(LINE_TYPE));
	_fmemcpy(lpSect->lpPointArray, lpData->PointArray, lpData->PointCount*sizeof(COLORPOINT_TYPE));
	CopyRect(&lpSect->DataRect, &lpData->glbBlock.wRect);
	DebugUnlockHandle(hData);
} /* end of FillLowData */

/*******************************************************************/
void SetSelword(HWND hWnd, LP_HLV lpHlv, int n)
{
	LP_HLV_SECT lpSect;
	RECT rc;

	if (lpHlv->lpCurSect == NULL)
	{
		return;
	}
	lpSect = lpHlv->lpCurSect;
	if (n == lpSect->selword)
	{
		return;
	}
	CopyRect(&rc, &(lpSect->lprcWords[n]));
	rc.right++;
	rc.bottom++;
	InvalidateRect(hWnd, &rc, TRUE);
	if (lpSect->selword >= 0)
	{
		CopyRect(&rc, &(lpSect->lprcWords[lpSect->selword]));
		rc.right++;
		rc.bottom++;
		InvalidateRect(hWnd, &rc, TRUE);
	}
	lpSect->selword = n;
	tapSetCurrentWord(n);
	InvalidateRect(hWnd, &(lpHlv->rcInk), TRUE);
	InvalidateRect(hWnd, &(lpHlv->rcXrs), TRUE);
} /* end of SetSelWord */

/*******************************************************************/
void FAR hlvSetCurSect(int curSect, int BranchSection)
{
	gCurSect = curSect;
	gInsertBranchSection = BranchSection;
} /* end of hlvSetCurSect */

/*******************************************************************/
void FAR hlvSetFirstWordNum(int firstWordNum)
{
	gFirstWordNum = firstWordNum;
} /* end of hlvSetCurSect */

/*******************************************************************/
BOOL FAR hlvPrepare(HWND hWnd, WORD PassNum)
{
	HGLOBAL     hHlv;
	LP_HLV      lpHlv;
	LP_HLV_SECT lpSect;
	BOOL        fRes;


	hHlv = (HGLOBAL) GetWindowLong(hWnd, GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);
	lpHlv->fValid = FALSE;
	lpHlv->fUpperDone = FALSE;
	fRes = FALSE;

	if (PassNum == 1)
	{
		// second pass
		lpSect = (LP_HLV_SECT) DebugAllocPtr(GSHND, sizeof(HLV_SECT_TYPE), "WGHLV hlvPrepare");
		if (lpSect == NULL || lpHlv->lpCurSect == NULL)
		{
			goto Error;
		}
		lpSect->index = lpHlv->lpCurSect->index;
		lpSect->infoType = lpHlv->lpCurSect->infoType;
		lpSect->PassNum = PassNum + 1;
		lpSect->sectionType = JUSTMADE_SECTION;
		// add to the end of section list
		InsertNewSection(lpHlv, lpSect, -1, TRUE);
		goto out;
	}

	if (gCurSect < 0)
	{
		/* Create new section */
		lpSect = (LP_HLV_SECT) DebugAllocPtr(GSHND, sizeof(HLV_SECT_TYPE), "WGHLV hlvPrepare");
		if (lpSect == NULL)
		{
			goto Error;
		}
		lpSect->index = -1;
		lpSect->infoType = -1; //SectionType;
		lpSect->PassNum = PassNum + 1;
		// add to the end of section list
		InsertNewSection(lpHlv, lpSect, -1, FALSE);
	}
	else
		if (gInsertBranchSection)
		{
			lpSect = (LP_HLV_SECT) DebugAllocPtr(GSHND, sizeof(HLV_SECT_TYPE), "WGHLV hlvPrepare");
			if (lpSect == NULL)
			{
				goto Error;
			}
			lpSect->index = gCurSect + gFirstWordNum;
			lpSect->infoType = SectionType;
			lpSect->PassNum = PassNum + 1;
			// add to the end of section list
			InsertNewSection(lpHlv, lpSect, lpSect->index, FALSE);
		}
		else
		{
			lpSect = FindSectByNumber(lpHlv, gFirstWordNum + gCurSect);
			if (lpSect == NULL)
			{
				goto Error;
			}
			lpSect->infoType = SectionType;
			lpSect->PassNum = PassNum + 1;
			if (lpSect->PassNum == 1)
			{
				KillBranchSections(hWnd, gFirstWordNum + gCurSect);
			}
		}
out:
	lpHlv->lpCurSect = lpSect;
	lpSect->sectionType = EMPTY_SECTION;
	if (PassNum == 1)
	{
		lpSect->sectionType = JUSTMADE_SECTION;
	}
	fRes = TRUE;
Error:
	DebugUnlockHandle(hHlv);
	return fRes;
} /* end of hlvPrepare */

/*******************************************************************/
void FAR hlvDone(HWND hWnd, BOOL LastWord, WORD PassNum)
{
	WORD        sectionType;
	HGLOBAL     hHlv;
	LP_HLV      lpHlv;
	LP_HLV_SECT lpSect;
	LPGUIDE     lpg;
	BOOL        FirstWord, Status;

	if (!IsWindow(hWnd)) /* ??AS check window class later */
	{
		return;
	}

	hHlv = (HGLOBAL) GetWindowLong(hWnd, GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);

	if (lpHlv->lpCurSect == NULL)
	{
		DebugUnlockHandle(hHlv);
		return;
	}
	if (!gWordCut)
	{
		lpg = recGetGuidesPtr();
	}
	else
		// if Word c*ut guides have no meaning
	{
		lpg = NULL;
	}
	lpSect = lpHlv->lpCurSect;

#ifndef _PENWIN
	{
		int   result;
		result = ReadRecognizerData(lpSect);
		if (result <= 0)
		{
			TrashSect(lpSect);
			DebugUnlockHandle(hHlv);
			return;
		}
	}
#endif

	FillTrace(lpSect, lpg, PassNum);
	lpHlv->fValid = TRUE;

	if (DebugLevel < 0 || gUseUpper)
	{
		inkDrawInit(hWnd);
		if (prfRunCompression())
		{
			CompressTrajectory(lpSect, IDM_COMPRESS);
			CompressTrajectory(lpSect, IDM_DECOMPRESS);
		}
		lpHlv->fUpperDone = TRUE;
		InvalidateRect(hWnd, NULL, TRUE);
		if (hAnswerDlgBox)
		{
			SetWindowsData(hAnswerDlgBox, ID_LETTERS, lpSect, TRUE, FALSE);
			InvalidateRect(hAnswerDlgBox, NULL, TRUE);
			UpdateWindow(hAnswerDlgBox);
		}
		else
			if (hCorrectDlgBox)
			{
				lpSect->selcor = 0;
				SetWindowsData(hCorrectDlgBox, ID_LETTERS, lpSect, TRUE, FALSE);
				InvalidateRect(hCorrectDlgBox, NULL, TRUE);
				UpdateWindow(hCorrectDlgBox);
			}
		sectionType = FILLED_SECTION;
	}
	else
	{
		sectionType = SEMIFILLED_SECTION;
		FillLowData(hWnd, lpSect);
	}

	for (lpSect = lpHlv->head; lpSect != NULL; lpSect = lpSect->next)
	{
		if (lpSect->sectionType == JUSTMADE_SECTION)
		{
			lpSect->sectionType = sectionType;
		}
	}
	DebugUnlockHandle(hHlv);
	fAutoEnlarge = TRUE;
	staGetStatus(ST_ONLINEINITREC, (LONG) ((BOOL FAR*)&Status));
	if (Status && prfRecordInk())
	{
		// only for on_line ink recognition
		staGetStatus(ST_FIRSTWORDFROMREC, (LONG) ((BOOL FAR *)&FirstWord));
		SendMessage(GetDlgItem(hLastDebugWnd, DRAWINK), WM_COMMAND, IDM_RECORD_TAP,
		            MAKELPARAM(FirstWord, 0));
	}
	staSetStatus(ST_FIRSTWORDFROMREC, (LPARAM) 0);
} /* end of hlvDone */

/******************************************************************************/
#ifndef _PENWIN

static int ReadGraphData(LP_HLV_SECT lpSect, BOOL AfterPP)
{
	RCBACCESS       lpfnRC;
	p_rec_info_type lpRCinfo;
	int             result, size;
	HINSTANCE       hDLL;

	RETURN_IF_BAD_POINTER(lpSect);
	hDLL = recGetDLLHandle();
	RETURN_IF_BAD_POINTER(hDLL);
	lpfnRC = (RCBACCESS) Q_GetProcAddress(hDLL, REC_DATA_ACCESS);
	RETURN_IF_BAD_POINTER(lpfnRC);
	lpRCinfo = recGetRCinfo();
	RETURN_IF_BAD_POINTER(lpRCinfo);

	// read graph
	if (!AfterPP)
	{
		size = (*lpfnRC)((void FAR *)lpRCinfo, OBJ_RWGRAPH, CMD_INF, 0, 0l);
		RETURN_IF_BAD_RESULT(size, 0);
		// alloc space  8 arrays :
		//  prwsb, pppdb - rws & ppd before postproccrsing
		//                         prwse, pppde - rws & ppd after postproccrsing
		//                         pselb, pnumb - auxilary arrays for PENLAB before postproccrsing
		//                         psele, pnume - auxilary arrays for PENLAB after postproccrsing
		FreeGraph(&lpSect->graph);
		result = AllocNewGraph(&lpSect->graph, size);
		RETURN_IF_BAD_RESULT(result, 0);
		lpSect->graph.size = size;
	}
	RETURN_IF_BAD_RESULT(lpSect->graph.size, 0);
	lpSect->graph.bafterpp = AfterPP;
	result = (*lpfnRC)((void FAR *)lpRCinfo,
	                   OBJ_RWGRAPH, CMD_GET, (UINT) lpSect->graph.size, (ULONG) &lpSect->graph);
	RETURN_IF_BAD_RESULT(result, 0);
	if (!AfterPP)
	{
		hlvSortGraph(GetDlgItem(hLastDebugWnd, DRAWINK));
	}
	return TRUE;
} /* end of ReadGraphData */

/******************************************************************************/
static int ReadRecognizerData(LP_HLV_SECT lpSect)
{
	RCBACCESS       lpfnRC;
	p_rec_info_type lpRCinfo;
	int             result;
	HINSTANCE       hDLL;

	RETURN_IF_BAD_POINTER(lpSect);
	hDLL = recGetDLLHandle();
	RETURN_IF_BAD_POINTER(hDLL);
	lpfnRC = (RCBACCESS) Q_GetProcAddress(hDLL, REC_DATA_ACCESS);
	RETURN_IF_BAD_POINTER(lpfnRC);
	lpRCinfo = recGetRCinfo();
	RETURN_IF_BAD_POINTER(lpRCinfo);

	// read trace
	lpSect->np_lo_trace = (*lpfnRC)((void FAR *)lpRCinfo, OBJ_TRAJECTORY, CMD_INF, 0, 0l);
	RETURN_IF_BAD_RESULT(lpSect->np_lo_trace, 0);
	DISPOSE(lpSect->lo_trace, "WGHLV ReadRecognizerData");
	lpSect->lo_trace = (LPPOINT) DebugAllocPtr(GSHND, lpSect->np_lo_trace*sizeof(LAB_PT_TYPE),
	                   "WGHLV ReadRecognizerData");
	RETURN_IF_BAD_POINTER(lpSect->lo_trace);
	result = (*lpfnRC)((void FAR *)lpRCinfo, OBJ_TRAJECTORY, CMD_GET,
	                   (UINT) lpSect->np_lo_trace, (ULONG) lpSect->lo_trace);
	RETURN_IF_BAD_RESULT(result, 0);

	// read line
	lineData.TDnp = (*lpfnRC)((void FAR *)lpRCinfo, OBJ_BASELINE, CMD_INF, 0, 0l);
	RETURN_IF_BAD_RESULT(lineData.TDnp, 0);
	DISPOSE(lineData.TDpt, "WGHLV ReadRecognizerData");
	lineData.TDpt = (LPPOINT) DebugAllocPtr(GSHND, lineData.TDnp*sizeof(short),
	                                        "WGHLV ReadRecognizerData");
	RETURN_IF_BAD_POINTER(lineData.TDpt);
	result = (*lpfnRC)((void FAR *)lpRCinfo, OBJ_BASELINE, CMD_GET,
	                   (UINT) lineData.TDnp, (ULONG) &lineData);
	RETURN_IF_BAD_RESULT(result, 0);

	if (DebugLevel >= 0)
	{
		return TRUE;
	}
	// read word records
	lpSect->n_rec_words = (*lpfnRC)((void FAR *)lpRCinfo, OBJ_RECWORDS, CMD_INF, 0, 0l);
	RETURN_IF_BAD_RESULT(lpSect->n_rec_words, 0);
	DISPOSE(lpSect->rec_words, "WGHLV ReadRecognizerData");
	lpSect->rec_words = (p_LAB_RECWORD)
	                    DebugAllocPtr(GSHND, lpSect->n_rec_words*sizeof(LAB_RECWORD_TYPE),
	                                  "WGHLV ReadRecognizerData");
	RETURN_IF_BAD_POINTER(lpSect->rec_words);
	result = (*lpfnRC)((void FAR *)lpRCinfo, OBJ_RECWORDS, CMD_GET,
	                   (UINT) lpSect->n_rec_words, (ULONG) lpSect->rec_words);
	//??SD    RETURN_IF_BAD_RESULT(result, 0);

	// read xr data
	lpSect->n_xrs = (*lpfnRC)((void FAR *)lpRCinfo, OBJ_XRDATA, CMD_INF, 0, 0l);
	RETURN_IF_BAD_RESULT(lpSect->n_xrs, 0);
	DISPOSE(lpSect->xrs, "WGHLV ReadRecognizerData");
	lpSect->xrs = (p_LAB_XRDATA) DebugAllocPtr(GSHND, lpSect->n_xrs*sizeof(LAB_XRDATA_TYPE),
	              "WGHLV ReadRecognizerData");
	RETURN_IF_BAD_POINTER(lpSect->lo_trace);
	result = (*lpfnRC)((void FAR *)lpRCinfo, OBJ_XRDATA, CMD_GET,
	                   (UINT) lpSect->n_xrs, (ULONG) lpSect->xrs);
	RETURN_IF_BAD_RESULT(result, 0);

	return TRUE;
} /* end of ReadRecognizerData */
#endif

/*******************************************************************/
void FAR hlvAllocFlTraceData(HWND hWnd, p_TRACEDATA pTraceData)
{
	HGLOBAL hHlv;
	LP_HLV  lpHlv;

	hHlv = (HGLOBAL) GetWindowLong(hWnd, GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);
	if (lpHlv->lpCurSect == NULL)
	{
		goto Error;
	}
	lpHlv->lpCurSect->lo_fl_trace = (LPPOINT)
	                                DebugAllocPtr(GSHND, pTraceData->TDnp*sizeof(POINT), "WGHLV hlvAllocFlTraceData");
	if (lpHlv->lpCurSect->lo_fl_trace == NULL)
	{
		goto Error;
	}
	pTraceData->TDpt = lpHlv->lpCurSect->lo_fl_trace;
	lpHlv->lpCurSect->np_lo_fl_trace = pTraceData->TDnp;
Error:
	DebugUnlockHandle(hHlv);
} /* end of hlvAllocFlTraceData */

/*******************************************************************/
void FAR hlvAllocTraceData(HWND hWnd, p_TRACEDATA pTraceData)
{
	HGLOBAL hHlv;
	LP_HLV  lpHlv;
#ifdef _PENWIN
	short    np;
	LPPOINT  lppt;
#endif

	hHlv = (HGLOBAL) GetWindowLong(hWnd, GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);
	if (lpHlv->lpCurSect == NULL)
	{
		goto Error;
	}
#ifdef _PENWIN
	if (lpHlv->lpCurSect->sectionType != EMPTY_SECTION)
	{
		/* It is possible only if wordcut flag is on
		  We still do not know how to process this situation,
		  how to fill guide and where place the results */
		FillTrace(lpHlv->lpCurSect, NULL, 0);
		lpHlv->lpCurSect = lpHlv->lpCurSect->next;
		if (lpHlv->lpCurSect == NULL)
		{
			goto Error;
		}
	}
	TrashSect(lpHlv->lpCurSect);
	lpHlv->lpCurSect->sectionType = JUSTMADE_SECTION;
	if (traceData.TDpt != NULL)
	{
		DebugFreePtr(traceData.TDpt, "WGHLV hlvAllocTraceData");
		traceData.TDnp = 0;
		traceData.TDpt = NULL;
	}
	np = pTraceData->TDnp;
	lppt = (LPPOINT)DebugAllocPtr(GSHND, np*sizeof(POINT), "WGHLV hlvAllocTraceData");
	traceData.TDpt = pTraceData->TDpt = lppt;
	traceData.TDnp = pTraceData->TDnp = np;
#else  // _PENWIN undefined
	TrashSect(lpHlv->lpCurSect);
	lpHlv->lpCurSect->sectionType = JUSTMADE_SECTION;
#endif
Error:
	DebugUnlockHandle(hHlv);
} /* end of hlvAllocTraceData */

/*******************************************************************/
void FAR hlvAllocGRData(HWND hWnd, p_GRAPHDATA pGRData)
{

	HGLOBAL       hHlv;
	LP_HLV        lpHlv;
	LP_HLV_SECT   lpSect;
	hHlv = (HGLOBAL) GetWindowLong(hWnd, GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);

	if (lpHlv->lpCurSect == NULL)
	{
		goto out;
	}
	lpSect = lpHlv->lpCurSect;
	ReadGraphData(lpSect, pGRData->after_pp);
out:
	DebugUnlockHandle(hHlv);
	return;
} /* end of hlvAllocGRData */

/*******************************************************************/
void FAR hlvAllocXRData(HWND hWnd, p_GETXRDATA pXRData)
{

#ifdef _PENWIN
	HGLOBAL hHlv;
	LP_HLV  lpHlv;
	LP_HLV_SECT lpSect;
	p_XR    lpxr;

	if (!bGetXRData)
	{
		return;
	}
	hHlv = (HGLOBAL) GetWindowLong(hWnd, GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);
	if (lpHlv->lpCurSect == NULL)
	{
		DebugUnlockHandle(hHlv);
		return;
	}
	lpSect = lpHlv->lpCurSect;
	if (prfShowGraph())
		// get only first request for xr, ignore all the others
	{
		bGetXRData = FALSE;
	}
	if (pXRData->nxr > 0)
	{
		lpxr = (p_XR)DebugAllocPtr(GSHND, pXRData->nxr*sizeof(XR_TYPE), "WGHLV hlvAllocXRData");
		lpSect->xrs  = pXRData->lpxr = lpxr;
		if (lpxr != NULL)
		{
			lpSect->n_xrs = pXRData->nxr;
		}
	}
#endif // _PENWIN

} /* end of hlvAllocXRData */

/*******************************************************************/
#if DEBUG_CHUNK_PROCESSOR
void FAR hlvAllocChunkData(HWND hWnd, p_LAB_CHUNK_DATA lpChunkAnswer)
{
	HGLOBAL hHlv;
	LP_HLV  lpHlv;
	LP_HLV_SECT lpSect;

	hHlv = (HGLOBAL) GetWindowLong(hWnd, GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);
	if (lpHlv->lpCurSect == NULL)
	{
		DebugUnlockHandle(hHlv);
		return;
	}
	lpSect = lpHlv->lpCurSect;
	lpSect->lpChunk = (p_LAB_CHUNK_DATA)DebugAllocPtr(GSHND, sizeof(LAB_CHUNK_DATA_TYPE),
	                  "WGHLV hlvAllocChunkData");
	if (lpSect->lpChunk != NULL)
		_fmemcpy((void FAR *)lpSect->lpChunk, (void FAR *)lpChunkAnswer,
		         sizeof(LAB_CHUNK_DATA_TYPE));
	DebugUnlockHandle(hHlv);
} /* end of hlvAllocChunkData */

/*******************************************************************/
void FAR hlvLetterPos(HWND hWnd, p_LAB_LETTERPOS lpLetterPos)
{
	HGLOBAL hHlv;
	LP_HLV  lpHlv;
	LP_HLV_SECT lpSect;

	hHlv = (HGLOBAL) GetWindowLong(hWnd, GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);
	if (lpHlv->lpCurSect == NULL)
	{
		DebugUnlockHandle(hHlv);
		return;
	}
	lpSect = lpHlv->lpCurSect;
	lpSect->lpLetterPos = (p_LAB_LETTERPOS)DebugAllocPtr(GSHND, sizeof(LAB_LETTERPOS_TYPE),
	                      "WGHLV hlvLetterPos");
	if (lpSect->lpLetterPos != NULL)
		_fmemcpy((void FAR *)lpSect->lpLetterPos, (void FAR *)lpLetterPos,
		         sizeof(LAB_LETTERPOS_TYPE));
	DebugUnlockHandle(hHlv);
} /* end of hlvLetterPos */
#endif

/*******************************************************************/
void FAR hlvAllocAnswerData(HWND hWnd, p_ANSWERDATA lpAnswers)
{

	//#ifdef _PENWIN
	HGLOBAL hHlv;
	LP_HLV  lpHlv;
	LP_HLV_SECT lpSect;

	hHlv = (HGLOBAL) GetWindowLong(hWnd, GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);

	if (lpHlv->lpCurSect == NULL)
	{
		DebugUnlockHandle(hHlv);
		return;
	}
	lpSect = lpHlv->lpCurSect;
	lpAnswers->answers = (p_ANSWER)
	                     DebugAllocPtr(GSHND, lpAnswers->nanswers*sizeof(ANSWER_TYPE),
	                                   "WGHLV hlvAllocAnswerData");
	lpSect->answers = (p_ANSWER) lpAnswers->answers;
	lpSect->n_answers = lpAnswers->nanswers;
	lpSect->lprcWords = (LPRECT) DebugAllocPtr(GSHND, lpAnswers->nanswers*sizeof(RECT),
	                    "WGHLV hlvAllocAnswerData");
	lpSect->fRcFilled = FALSE;
	DebugUnlockHandle(hHlv);
	//#endif
} /* end of hlvAllocAnswerData */

/*******************************************************************/
void FAR hlvAllocRecWordsData(HWND hWnd, p_RECWORDSDATA lpRecWords)
{
	HGLOBAL hHlv;
	LP_HLV  lpHlv;
	LP_HLV_SECT lpSect;

	hHlv = (HGLOBAL) GetWindowLong(hWnd, GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);

	if (lpHlv->lpCurSect == NULL)
	{
		DebugUnlockHandle(hHlv);
		return;
	}
	lpSect = lpHlv->lpCurSect;
	DISPOSE(lpSect->rec_words, "WGHLV hlvAllocRecWordsData");
#ifdef _PENWIN
	lpRecWords->words = (p_rec_w_type)
	                    DebugAllocPtr(GSHND, lpRecWords->nwords*sizeof(rec_w_type),
	                                  "WGHLV hlvAllocRecWordsData");
#else
	lpRecWords->words = (void FAR *)
	                    DebugAllocPtr(GSHND, lpRecWords->nwords*sizeof(LAB_RECWORD_TYPE),
	                                  "WGHLV hlvAllocRecWordsData");
#endif
	lpSect->rec_words = (p_LAB_RECWORD) lpRecWords->words;
	lpSect->n_rec_words = lpRecWords->nwords;
	DebugUnlockHandle(hHlv);
} /* end of hlvAllocRecWordsData */

/*******************************************************************/
void FAR hlvAllocLineData(HWND hWnd, p_TRACEDATA pLineData)
{
#ifdef _PENWIN
	short   np;
	LPPOINT lppt;

	if (lineData.TDpt != NULL)
	{
		DebugFreePtr(lineData.TDpt, "WGHLV hlvAllocLineData");
		lineData.TDnp = 0;
		lineData.TDpt = NULL;
	}
	np = pLineData->TDnp;
	lppt = (LPPOINT)DebugAllocPtr(GSHND, np*2, "WGHLV hlvAllocLineData"); /* 2 bytes per point */
	lineData.TDpt = pLineData->TDpt = lppt;
	lineData.TDnp = pLineData->TDnp = np;
#endif
} /* end of hlvAllocLineData */


#if DEBUG_CHUNK_PROCESSOR
/*******************************************************************/
/* The following function actions are tightly coupled with their   */
/* counterparts in the guts of chunk processor, namely LC_IsEmpty, */
/* LC_Unpack and LETTER_CHAR. If any of beforementioned functions  */
/* will be changed you MUST change appropriate parts of this code. */
/*******************************************************************/

/* Following three defines are literal copy from _lowobj.h         */

#define LC_OFF   ('Z' - 'A' + 1)

#define LETTER_CHAR(n) (_UCHAR)                                      \
(                                                                    \
  (n) > 0 && (n) < LC_OFF + 1 ? 'A' + (n) :                          \
  (n) >= LC_OFF + 1 && (n) < 2*LC_OFF + 1 ? 'a' - LC_OFF - 1 + (n) : \
  0                                                                  \
)

#define LOSC_NO 0xffff

/* Following two functions are almost literal copy from _lowobj.c  */

/*******************************************************************/
static _INT LC_Unpack(p_LAB_LC p_lc, p_UCHAR szBuf, _INT nBufSize)
{
	_INT   iBufCur=0,i;
	_UCHAR ch;
	LAB_LC_TYPE lcopy;

	//Make copy of the container.
	lcopy=*p_lc;

	for(i=0; i<26; i++)
	{
		if(lcopy.ulSmLet&((_ULONG)1))
		{
			ch = (_UCHAR) ('a'+i);
			szBuf[iBufCur]=ch;
			iBufCur++;
			if(nBufSize-1==iBufCur)
			{
				szBuf[iBufCur]='\0';
				return iBufCur;
			}
		}
		lcopy.ulSmLet>>=1;
	}
	for(i=0; i<26; i++)
	{
		if(lcopy.ulCpLet&((_ULONG)1))
		{
			ch = (_UCHAR) ('A'+i);
			szBuf[iBufCur]=ch;
			iBufCur++;
			if(nBufSize-1==iBufCur)
			{
				szBuf[iBufCur]='\0';
				return iBufCur;
			}
		}
		lcopy.ulCpLet>>=1;
	}
	szBuf[iBufCur]='\0';
	return iBufCur;
}//LC_Unpack.

static _BOOL LC_IsEmpty(p_LAB_LC p_lc)
{
	if(p_lc->ulSmLet!=0L||p_lc->ulCpLet!=0L)
	{
		return _FALSE;
	}
	else
	{
		return _TRUE;
	}
} //LC_IsEmpty.

/*******************************************************************/
static BOOL UnpackLettersFromLO(p_LAB_LOWOBJ pLO, LPSTR buf, int size)
{
	int nCase, nCode;

	nCase = (pLO->wAttr-800)/100;
	nCode = pLO->wAttr-800-nCase*100;
	if(nCode == 0)
	{
		if(LC_IsEmpty(&(pLO->lc)))
		{
			return FALSE;
		}
		LC_Unpack(&(pLO->lc), buf, size);
	}
	else
		if(nCode <= LC_OFF*2)
		{
			buf[0] = LETTER_CHAR(nCode);
			buf[1] = '\0';
		}
		else
		{
			return FALSE;
		}
	return TRUE;
} /* UnpackLettersFromLO */

/*******************************************************************/
BOOL FAR hlvPaintChunkData(HWND hWnd, HDC hDC)
{
	HGLOBAL     hHlv;
	LP_HLV      lpHlv;
	LP_HLV_SECT lpSect;
	p_LAB_CHUNK_DATA lpChunk;
	BOOL        result = TRUE;
	RECT        rcv, rcInk, rcChunk, rcAnswer, rcTrace;
	GLB_DATA_TYPE glb;
	LOGFONT     lf;
	HFONT       hFont, hOldFont;
	HPEN        hPen, hPenOld;
	int         i, a_height, i_height, len;
	char        word[LAB_RW_SIZE], newword[2*LAB_RW_SIZE];

	hHlv = (HGLOBAL) GetWindowLong(hWnd, GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);
	if (lpHlv->lpCurSect == NULL || lpHlv->lpCurSect->lpChunk == NULL)
	{
		goto Unlock;
	}
	lpSect = lpHlv->lpCurSect;
	lpChunk = lpSect->lpChunk;
	UpdateSectNumber(lpSect);
	if (lpSect->sectionType != FILLED_SECTION)
	{
		result = FALSE;
		goto Unlock;
	}
	GetClientRect(hWnd, &rcv);
	glbInit(hWnd, NULL, &glb);
	hPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 0));
	if (hPen == NULL)
	{
		goto Unlock;
	}
	hPenOld = SelectObject(hDC, hPen);
	SelectObject(hDC, GetStockObject(ANSI_FIXED_FONT));
	GetObject(GetStockObject(ANSI_FIXED_FONT), sizeof(LOGFONT), &lf);
	a_height = 2*lf.lfHeight;
	i_height = (rcv.bottom - rcv.top - a_height > 0) ?
	           ((rcv.bottom - rcv.top - a_height)/2) : 0;
	a_height = rcv.bottom - rcv.top - 2*i_height;
	SetRect(&rcInk, rcv.left, rcv.top, rcv.right, rcv.top + i_height);
	if (DebugLevel == -300)
	{
		SetRect(&rcChunk, rcv.left, rcInk.bottom, rcv.right, rcInk.bottom + i_height);
		SetRect(&rcAnswer, rcv.left, rcChunk.bottom, rcv.right, rcv.bottom);
	}
	else
	{
		SetRect(&rcAnswer, rcv.left, rcInk.bottom, rcv.right, rcv.bottom);
	}
	MoveToEx(hDC, rcv.left,  rcInk.bottom, NULL);
	LineTo(hDC, rcv.right, rcInk.bottom);
	if (DebugLevel == -300)
	{
		MoveToEx(hDC, rcv.left,  rcChunk.bottom, NULL);
		LineTo(hDC, rcv.right, rcChunk.bottom);
	}
	SelectObject(hDC, hPenOld);
	DeleteObject(hPen);
	// draw trajectory
	if (lpSect->trace.nPoints > 0)
	{
		trcMeasureWord(&(lpSect->trace), 0, &rcTrace);
		glbWindow(&glb, &rcTrace);
		InflateRect(&rcInk, -2, -2);
		glbViewport(&glb, &rcInk);
		DrawTrace(hDC, &glb, lpSect, FALSE, FALSE);
		DrawObjects(hDC, &glb, lpSect);
	}
	// draw chunks
	if (lpChunk->nChNumb > 0 && DebugLevel == -300)
	{
		trcMeasureWord(&(lpSect->trace), 0, &rcTrace);
		glbWindow(&glb, &rcTrace);
		InflateRect(&rcChunk, -2, -2);
		glbViewport(&glb, &rcChunk);
		DrawChunk(hDC, &glb, lpChunk);
	}
	// draw answer
	lf.lfHeight = lf.lfHeight*2-2;
	lstrcpy(lf.lfFaceName, "Courier New");
	hFont = CreateFontIndirect(&lf);
	if (hFont)
	{
		hOldFont = SelectObject(hDC, hFont);
	}
	if (DebugLevel == -300)
	{
		lstrcpy(word, lpChunk->szAnswer);
		len = lstrlen(word);
		for(i = 0; i < len; i++)
		{
			if (word[i] == '\xff')
			{
				word[i] = '?';
			}
		}
		prfEncode(word, newword, TRUE);
		SetBkMode(hDC, TRANSPARENT);
		if (lpChunk->fIsNumber)
		{
			DrawChunkAnswer(hDC, lpSect, &rcAnswer, newword);
		}
		else
		{
			lstrcat(newword, " - is word!");
			SetTextColor(hDC, RGB(255,0,0));
			DrawText(hDC, newword, lstrlen(newword), &rcAnswer, DT_LEFT | DT_VCENTER);
		}
	}
	else
		if (DebugLevel == -301)
		{
			p_LAB_LOWOBJ    lpLoObject;
			LPPOINT         lppPoints;
			int             i, j, s_beg, s_end;
			int             x, y, xmin, xmax, xavg;
			char            buf[16]; // I think it is more than enough
			COLORREF        Color[4];
			SIZE            size;
			RECT            rc;

			Color[0] = RGB(0, 255, 0);     /* Green */
			Color[1] = RGB(0, 0, 255);     /* Blue  */
			Color[2] = RGB(255, 0, 0);     /* Red   */
			Color[3] = RGB(255, 255, 255); /* White */
			lppPoints = lpSect->lo_trace;
			lpLoObject = lpSect->lpChunk->pLO;
			glbWindow(&glb, &rcTrace);
			glbViewport(&glb, &rcInk);
			for (i = 0; i < lpChunk->nLONumb; i++)
			{
				/* Find horisontal bounds of trajectory of this object */
				if (lpLoObject[i].wAttr == LOSC_NO)
				{
					continue;
				}
				s_beg = lpLoObject[i].iBegTr;
				s_end = lpLoObject[i].iEndTr;
				xmin = INT_MAX;
				xmax = INT_MIN;
				for (j = s_beg; j < s_end; j++)
				{
					if (lppPoints[j].y != -1)
					{
						if (lppPoints[j].x < xmin)
						{
							xmin = lppPoints[j].x;
						}
						if (lppPoints[j].x > xmax)
						{
							xmax = lppPoints[j].x;
						}
					}
				}
				/* Translate it into viewport coordinate system */
				glbWindowToViewport(&glb, xmin, 0, &x, &y);
				xmin = x;
				glbWindowToViewport(&glb, xmax, 0, &x, &y);
				xmax = x;
				xavg = (xmin + xmax) / 2;
				/* Unpack letter(s) from very complicated format */
				UnpackLettersFromLO(lpLoObject+i, buf, sizeof(buf));
				/* Draw letter(s) for this object underneath it */
				SetBkColor(hDC, RGB(0, 0, 0));
				SetTextColor(hDC, Color[i%4]);
				SetRect(&rc, xavg-50, rcAnswer.top + 2, xavg+50, rcAnswer.bottom);
				for (j = 0; buf[j] != '\0'; j++)
				{
					DrawText(hDC, buf+j, 1, &rc, DT_CENTER);
					GetTextExtentPoint(hDC, buf+j, 1, &size);
					rc.top += size.cy + 2;
				}
			}
		}
	if (hFont)
	{
		SelectObject(hDC, hOldFont);
		DeleteObject(hFont);
	}
Unlock:
	DebugUnlockHandle(hHlv);
	return result;
} /* end of hlvPaintChunkData */

#endif  // DEBUG_CHUNK_PROCESSOR
/*******************************************************************/
BOOL FAR hlvPaint(HWND hWnd, HDC hDC)
{
	HGLOBAL       hHlv;
	LP_HLV        lpHlv;
	LP_HLV_SECT   lpSect;
	RECT          rcv, rcw, rcTrace, rcInk, rcXrs, rcAnswer, rcPAnswer, rcGraph;
	GLB_DATA_TYPE glb;
	HPEN          hPen, hPenOld;
	long          h, w, hanswr, hxrs, hink, w1;
	BOOL          result, symb_graph = 0, show_compression;
	BOOL          Status;

	result = FALSE;
#if DEBUG_CHUNK_PROCESSOR
	if (DebugLevel <= -300)
	{
		result = hlvPaintChunkData(hWnd, hDC);
		fAutoEnlarge = FALSE;
		return result;
	}
#endif
	hPen = CreatePen(PS_SOLID, 1, RGB(255, 128, 0));
	if (hPen == NULL)
	{
		goto Error;
	}

	hHlv = (HGLOBAL) GetWindowLong(hWnd, GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);
	if (lpHlv->lpCurSect == NULL)
	{
		goto Unlock;
	}
	lpSect = lpHlv->lpCurSect;
	UpdateSectNumber(lpSect);

	GetClientRect(hWnd, &rcv);
	glbInit(hWnd, NULL, &glb);

	if (lpSect->sectionType != FILLED_SECTION)
	{
		staGetStatus(ST_ZOOM, (LONG) (BOOL FAR *)&Status);
		if (lpSect->PointCount > 0 && !Status)
		{
			glbWindow(&glb, &lpSect->DataRect);
			glbViewport(&glb, &rcv);
			DrawLowLevelData(hWnd, hDC, &glb, lpSect);
			result = TRUE;
		}
		goto Unlock;
	}

	hPenOld = (HPEN) SelectObject(hDC, hPen);

	show_compression = prfRunCompression();

#define HANSWER 220
#define HXRS    150
#define HINK    100
#define HTOTAL (HANSWER + HXRS + HINK)
#define HMINIMUM (HANSWER * 2 / 3 + HXRS + HINK/2)

	if (!show_compression)
	{
		w = rcv.right - rcv.left;
		h = rcv.bottom - rcv.top;
		rcInk.left = rcXrs.left = rcAnswer.left = rcv.left;
		rcInk.right = rcXrs.right = rcAnswer.right = rcv.right;
		hanswr = h * HANSWER / HTOTAL;
		hxrs = h * HXRS / HTOTAL;
		hink = h - hanswr - hxrs;

		//??SD
		rcInk.top = rcv.top;
		rcInk.bottom = rcInk.top + (int) hink;
		rcXrs.top = rcInk.bottom;
		rcXrs.bottom = rcXrs.top + (int) hxrs;
		rcAnswer.top = rcPAnswer.top = rcGraph.top = rcXrs.bottom;
		rcAnswer.bottom = rcPAnswer.bottom = rcGraph.bottom = rcv.bottom;
		rcAnswer.left = rcInk.left;
		rcPAnswer.right = rcInk.right;
		symb_graph = (prfShowGraph() && lpSect->graph.type == LAB_RWGT_SEPLET);
		if (symb_graph)
		{
			w1 = LOWORD(w) / 3;
			rcAnswer.right = rcAnswer.left + (w - w1) * 6 / 10;
			rcGraph.left = rcAnswer.right;
			rcGraph.right = rcPAnswer.left = rcGraph.left + w1;
		}
		else
		{
			rcGraph.left = rcGraph.right = rcAnswer.right;
			rcAnswer.right = rcAnswer.left + w * 6 / 10;
			rcPAnswer.left = rcAnswer.right;
		}
		CopyRect(&(lpHlv->rcInk), &rcInk);
		CopyRect(&(lpHlv->rcXrs), &rcXrs);
		CopyRect(&(lpHlv->rcAnswer), &rcAnswer);
		CopyRect(&(lpHlv->rcPAnswer), &rcPAnswer);
		CopyRect(&(lpHlv->rcGraph), &rcGraph);
	}
	else
	{
		rcInk.left = rcAnswer.left = rcv.left;
		rcInk.right = rcAnswer.right = rcv.right;
		rcInk.top = rcv.top;
		rcInk.bottom = rcInk.top + (rcv.bottom - rcv.top) * 4 / 5;
		rcAnswer.top = rcInk.bottom;
		rcAnswer.bottom = rcv.bottom;
		CopyRect(&(lpHlv->rcInk), &rcInk);
		CopyRect(&(lpHlv->rcAnswer), &rcAnswer);
	}

	SetBkMode(hDC, TRANSPARENT);
	if (lpSect->trace.nPoints > 0)
	{
		trcMeasureWord(&(lpSect->trace), 0, &rcTrace);
		glbWindow(&glb, &rcTrace);
		InflateRect(&rcInk, -2, -2);
		glbViewport(&glb, &rcInk);
		DrawLine(hDC, &glb, lpSect);
		DrawTrace(hDC, &glb, lpSect, symb_graph, show_compression);
	}

	if (show_compression)
	{
		DrawCompressionResult(hDC, lpSect, lpHlv);
		goto out;
	}
	SetRect(&rcw, 0, 0, 1000, (int) (1000L * hxrs / w));
	glbWindow(&glb, &rcw);
	glbViewport(&glb, &rcXrs);

	glbMoveTo(&glb, hDC, 0, 0);
	glbLineTo(&glb, hDC, 1000, 0);

	DrawXr(hDC, lpSect, &rcXrs, symb_graph);

	DrawAnswer(hDC, lpSect, lpHlv, symb_graph);

	if (prfDebugOn())
	{
		DrawSections(hDC, lpHlv, &rcInk);
	}
	result = TRUE;

out:
	SelectObject(hDC, hPenOld);

Unlock:
	DebugUnlockHandle(hHlv);

Error:
	if (hPen != NULL)
	{
		DeleteObject(hPen);
	}
	fAutoEnlarge = FALSE;
	return result;
} /* end of hlvPaint */

/*******************************************************************/
BOOL FAR hlvCreate(HWND hWnd)
{
	HGLOBAL hHlv;
	LP_HLV  lpHlv;
	int     nWords;

	hHlv = DebugAlloc(GHND, sizeof(HLV_TYPE), "WGHLV hlvCreate");
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);
	lpHlv->nSects = 0;
	nWords = tapGetTAPWordsCount(hLastTAPWnd);
	if (nWords != 0)
	{
		BuildSectionList(lpHlv, nWords);
	}
	DebugUnlockHandle(hHlv);
	SetWindowLong(hWnd, GWW_HLV_DATA, (LONG) hHlv);
	return hHlv != NULL;
} /* end of hlvCreate */

/*******************************************************************/
BOOL FAR hlvDestroy(HWND hWnd)
{
	HGLOBAL hHlv;
	LP_HLV lpHlv;

	hHlv = (HGLOBAL) GetWindowLong(hWnd, GWW_HLV_DATA);
	if (hHlv != NULL)
	{
		lpHlv = (LP_HLV) DebugLockHandle(hHlv);
		TrashAll(lpHlv, TRUE);
		DebugUnlockHandle(hHlv);
		hHlv = DebugFree(hHlv, "WGHLV hlvDestroy");
		SetWindowLong(hWnd, GWW_HLV_DATA, (LONG) hHlv);
	}
	return TRUE;
} /* end of hlvDestroy */

/*******************************************************************/
BOOL FAR hlvKEYDOWN(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HGLOBAL       hHlv;
	LP_HLV        lpHlv;
	LP_HLV_SECT   lpSect;
	BOOL          result, symb_graph;
	p_GRAPH_TYPE graph;
	int           n;

	result = FALSE;
	hHlv = (HGLOBAL) GetWindowLong(hWnd, GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);
	if (lpHlv->lpCurSect == NULL)
	{
		goto Unlock;
	}
	if (!lpHlv->fUpperDone)
	{
		goto Unlock;
	}
	lpSect = lpHlv->lpCurSect;
	symb_graph = (prfShowGraph() && lpSect->graph.type == LAB_RWGT_SEPLET);
	if (!symb_graph)
	{
		if (lpSect->answers == NULL)
		{
			DebugUnlockHandle(hHlv);
			return FALSE;
		}
		/*if (wParam == SB_LINEDOWN) {
		   n = lpSect->selword + 1;
		   if (n >= lpSect->n_answers || lpSect->answers[n].anword[0] == '\0') n = 0;
		   }
		   else if (wParam == SB_LINEUP){
		   n = lpSect->selword - 1;
		   if (n < 0) {
		   n = lpSect->n_answers - 1;
		   while (n > 0 && lpSect->answers[n].anword[0] == '\0')
		   n--;
		   }
		   }
		   else*/
		{
			DebugUnlockHandle(hHlv);
			return FALSE;
		}
		SetSelword(hWnd, lpHlv, n);
		result = TRUE;
	}
	else
	{
		graph = &lpSect->graph;
		if (wParam == SB_LINEDOWN)
		{
			if (LOWORD(lParam) == VK_RIGHT)
			{
				n = GetSelectedChar(graph, VK_RIGHT);
			}
			else
			{
				n = GetSelectedChar(graph, VK_DOWN);
			}
		}
		else
			if (wParam == SB_LINEUP)
			{
				if (LOWORD(lParam) == VK_LEFT)
				{
					n = GetSelectedChar(graph, VK_LEFT);
				}
				else
				{
					n = GetSelectedChar(graph, VK_UP);
				}
			}
			else
			{
				DebugUnlockHandle(hHlv);
				return FALSE;
			}
		SelectNewChar(hWnd, lpSect, lpHlv, 0L, n);
		result = TRUE;
	}
Unlock:
	DebugUnlockHandle(hHlv);
	return result;
} /* end of hlvKEYDOWN */

/*******************************************************************/
void FAR hlvResetXRRectangles(HWND hWnd)
{
	HGLOBAL     hHlv;
	LP_HLV      lpHlv;

	hHlv = (HGLOBAL) GetWindowLong(hWnd, GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);
	if (lpHlv != NULL && lpHlv->lpCurSect != NULL && lpHlv->lpCurSect->pxrrect != NULL)
	{
		lpHlv->lpCurSect->pxrrect[0].left = lpHlv->lpCurSect->pxrrect[0].right = 0;
	}
	DebugUnlockHandle(hHlv);
} /**/

BOOL hlvStartXrInputSelection(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HGLOBAL     hHlv;
	LP_HLV      lpHlv;
	LP_HLV_SECT lpSect;
	POINT       pt;
	int         nxr;
	BOOL        processed = FALSE;

	if (DebugLevel >= 0)
	{
		return processed;
	}
	hHlv = (HGLOBAL) GetWindowLong(hWnd, GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);

	if (lpHlv->lpCurSect == NULL || lpHlv->lpCurSect->pxrrect_input == NULL)
	{
		goto Unlock;
	}
	lpSect = lpHlv->lpCurSect;
	pt.x = LOWORD(lParam);
	pt.y = HIWORD(lParam);

	lpSect->range_begin = lpSect->range_end = -1;
	// check if XR element was hit
	if (PtInRect(&(lpHlv->rcXrs), pt) && lpSect->pxrrect_input != NULL)
	{
		nxr = 0;
		while (lpSect->pxrrect_input[nxr].left != 0 &&
		        lpSect->pxrrect_input[nxr].right != 0)
		{
			if (PtInRect(&(lpSect->pxrrect_input[nxr]), pt))
			{
				// show input XR
				lpSect->range_begin = nxr;
				break;
			}
			nxr++;
		}
	}
	processed = TRUE;
Unlock:
	DebugUnlockHandle(hHlv);
	return processed;
}


BOOL MakeLrnParam(LP_HLV_SECT lpSect, LrnParam* param)
{
	param->xrs = lpSect->xrs + lpSect->range_begin;
	param->n_xrs = lpSect->range_end - lpSect->range_begin;
	return TRUE;
}

BOOL hlvEndXrInputSelection(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HGLOBAL     hHlv;
	LP_HLV      lpHlv;
	LP_HLV_SECT lpSect;
	POINT       pt;
	int         nxr;

	if (DebugLevel >= 0)
	{
		return FALSE;
	}
	hHlv = (HGLOBAL) GetWindowLong(hWnd, GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);
	if (lpHlv->lpCurSect == NULL || lpHlv->lpCurSect->pxrrect_input == NULL)
	{
		return FALSE;
	}
	lpSect = lpHlv->lpCurSect;

	if (lpSect->range_begin == -1)
	{
		DebugUnlockHandle(hHlv);
		return FALSE;
	}

	pt.x = LOWORD(lParam);

	int bound[160] = { 0, };
	nxr = 1;
	while (lpSect->pxrrect_input[nxr].left != 0 &&
	        lpSect->pxrrect_input[nxr].right != 0)
	{
		bound[nxr] = (lpSect->pxrrect_input[nxr - 1].right + lpSect->pxrrect_input[nxr].left) / 2;
		++nxr;
	}

	bound[nxr] = 32767;

	for (int i = 0; i < nxr; i++)
	{
		if (pt.x < bound[i + 1])
		{
			lpSect->range_end = i;
			break;
		}
	}

	if (lpSect->range_begin > lpSect->range_end)
	{
		int tmp = lpSect->range_begin;
		lpSect->range_begin = lpSect->range_end;
		lpSect->range_end = tmp;
	}

	int num_segment = lpSect->range_end - lpSect->range_begin + 1;

	if (num_segment > 3)
	{
		LrnParam param;
		MakeLrnParam(lpSect, &param);
		createInteractiveLearn(hWnd, &param);
	}
	DebugUnlockHandle(hHlv);
	return TRUE;
}


/*******************************************************************/
BOOL FAR hlvShowXRInput(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HGLOBAL     hHlv;
	LP_HLV      lpHlv;
	LP_HLV_SECT lpSect;
	POINT       pt;
	int         nxr;

	if (DebugLevel >= 0)
	{
		return FALSE;
	}
	hHlv = (HGLOBAL) GetWindowLong(hWnd, GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);

	if (lpHlv->lpCurSect == NULL)
	{
		goto Unlock;
	}
	lpSect = lpHlv->lpCurSect;
	pt.x = LOWORD(lParam);
	pt.y = HIWORD(lParam);

	// check if XR element was hit
	if (PtInRect(&(lpHlv->rcXrs), pt) && lpSect->pxrrect_input != NULL)
	{
		nxr = 0;
		while (lpSect->pxrrect_input[nxr].left != 0 &&
		        lpSect->pxrrect_input[nxr].right != 0)
		{
			if (PtInRect(&(lpSect->pxrrect_input[nxr]), pt))
			{
				// show input XR
				ShowXRInput(lpSect, nxr, pt);
				break;
			}
			nxr++;
		}
	}
Unlock:
	DebugUnlockHandle(hHlv);
	return TRUE;
} /* end of BOOL FAR hlvShowXRInput() */


void GetSymVarId(LP_HLV_SECT lpSect, int nxr, wchar_t* sym, int* var, int* time)
{
	int n, k;
	int count = 0;

	for (n = 0; n < LAB_RW_SIZE && lpSect->ppd[n].letter != 0; n++)
	{
		for (k = 0; k < LAB_XR_SIZE && lpSect->ppd[n].xr[k] != 0; k++)
		{
			if (count >= nxr)
			{
				*sym = lpSect->ppd[n].letter;
				*var = lpSect->ppd[n].num_var;
				*time = k;
				return;
			}
			count++;
		}
	}
}

/*******************************************************************/
BOOL FAR hlvShowXRCorrelation(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HGLOBAL     hHlv;
	LP_HLV      lpHlv;
	LP_HLV_SECT lpSect;
	POINT       pt;
	int         nxr;

	if (DebugLevel >= 0)
	{
		return FALSE;
	}
	hHlv = (HGLOBAL) GetWindowLong(hWnd, GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);

	if (lpHlv->lpCurSect == NULL)
	{
		goto Unlock;
	}
	lpSect = lpHlv->lpCurSect;
	pt.x = LOWORD(lParam);
	pt.y = HIWORD(lParam);

	// check if XR element was hit
	if (PtInRect(&(lpHlv->rcXrs), pt) && lpSect->pxrrect != NULL)
	{
		nxr = 0;
		while (lpSect->pxrrect[nxr].left != 0 && lpSect->pxrrect[nxr].right != 0)
		{
			if (PtInRect(&(lpSect->pxrrect[nxr]), pt))
			{
				// show matrix result
				if (wParam & MK_CONTROL)
				{
					wchar_t sym;
					int var, time;
					GetSymVarId(lpSect, nxr, &sym, &var, &time);
					tplCreateTemplateDialog(hWnd, sym, var, time);
				}
				else
				{
					ShowXRCorrelation(lpSect, nxr, pt);
				}
				break;
			}
			nxr++;
		}
	}
Unlock:
	DebugUnlockHandle(hHlv);
	return TRUE;
} /* end of hlvShowXRCorrelation() */

/*******************************************************************/
BOOL FAR hlvLBUTTONDOWN(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HGLOBAL     hHlv;
	LP_HLV      lpHlv;
	LP_HLV_SECT lpSect;
	int         n;
	BOOL        result, found, symb_graph;
	POINT       pt;

	if (DebugLevel >= 0)
	{
		return FALSE;
	}
	result = FALSE;
	hHlv = (HGLOBAL) GetWindowLong(hWnd, GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);

	if (lpHlv->lpCurSect == NULL)
	{
		goto Unlock;
	}
	lpSect = lpHlv->lpCurSect;
	pt.x = LOWORD(lParam);
	pt.y = HIWORD(lParam);
	if (PtInRect(&(lpHlv->rcAnswer), pt))
	{
		result = TRUE;
	}
	symb_graph = (prfShowGraph() && lpSect->graph.type == LAB_RWGT_SEPLET);

	if (!symb_graph)
	{
		found = FALSE;
		if (lpSect->lprcWords != NULL)
		{
			for (n = 0; n < lpSect->n_answers; n++)
			{
				if (PtInRect(&(lpSect->lprcWords[n]), pt))
				{
					found = TRUE;
					break;
				}
			}
		}
		if (found)
		{
			SetSelword(hWnd, lpHlv, n);
		}
		result = found;
	}
	else
	{
		result = SelectNewChar(hWnd, lpSect, lpHlv, lParam, 0);
	}
Unlock:
	DebugUnlockHandle(hHlv);
	return (result);
} /* end of hlvLBUTTONDOWN */

/*********************************************************/
BOOL FAR hlvGetXrData(HWND hInkWnd, p_MPRINT pMP)
{
	HGLOBAL hHlv;
	LP_HLV  lpHlv;
	LP_HLV_SECT lpSect;

	RETURN_IF_BAD_POINTER(pMP);
	RETURN_IF_BAD_POINTER(hInkWnd);
	hHlv = (HGLOBAL) GetWindowLong(hInkWnd, GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);
	if (!lpHlv->fValid || lpHlv->lpCurSect == NULL)
	{
		pMP->MPnxr = 0;
		pMP->MPxr = NULL;
		DebugUnlockHandle(hHlv);
		return FALSE;
	}
	lpSect = lpHlv->lpCurSect;
	pMP->MPnxr = lpSect->n_xrs;
	pMP->MPxr = lpSect->xrs;
	DebugUnlockHandle(hHlv);
	return TRUE;
} /* end of hlvGetXrData */

/*********************************************************/
BOOL FAR hlvGetSelWord(HWND hInkWnd, LPSTR lpsz, int cb)
{
	HGLOBAL     hHlv;
	LP_HLV      lpHlv;
	int         result = FALSE;

	RETURN_IF_BAD_POINTER(lpsz);
	hHlv = (HGLOBAL) GetWindowLong(hInkWnd, GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);
	if (lpHlv->fUpperDone && lpHlv->lpCurSect != NULL)
	{
		lstrcpy(lpsz, lpHlv->lpCurSect->answers[lpHlv->lpCurSect->selword].anword);
		result = TRUE;
	}
	DebugUnlockHandle(hHlv);
	return result;
} /* end of hlvGetSelWord */

/*********************************************************/
void FAR hlvCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	HGLOBAL     hHlv;
	LP_HLV      lpHlv;
	LP_TRACE    lpTrace;
	LP_HLV_SECT lpSect;
	WORD        id;

	hHlv = (HGLOBAL) GetWindowLong(hWnd, GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);
	lpSect = lpHlv->lpCurSect;
	id = GET_WM_COMMAND_ID(wParam, lParam);
	if (((!lpHlv->fValid) || lpHlv->head == NULL) && id != IDM_REC_CONFIG)
	{
		goto Error;
	}
	switch (id)
	{
		case IDM_PREV_WORD:
		case IDM_NEXT_WORD:
			lpSect = FindNextWord(lpHlv, id == IDM_NEXT_WORD);
			if (lpSect != lpHlv->lpCurSect)
			{
				lpHlv->lpCurSect = lpSect;
				if (lpSect->index >= 0 && lpSect->sectionType == EMPTY_SECTION)
				{
					//  recognize lpSect->index'th word of TAP file
					tapRecognizeWord(hLastTAPWnd, lpSect->index, FALSE);
				}
				else
					if (lpSect->sectionType != FILLED_SECTION)
					{
						lpTrace = &(lpSect->trace);
						gFirstWordNum = FindSectNumber(lpHlv, lpSect);
						gCurSect = 0;
						if (gFirstWordNum >= 0)
						{
							recRecognizeTrace(lpTrace, NULL, FALSE, FALSE, gFirstWordNum);
						}
					}
				InvalidateRect(hWnd, NULL, TRUE);
				if (hAnswerDlgBox)
				{
					SetWindowsData(hAnswerDlgBox, ID_LETTERS, lpSect, TRUE, FALSE);
					InvalidateRect(hAnswerDlgBox, NULL, TRUE);
					UpdateWindow(hAnswerDlgBox);
				}
				else
					if (hCorrectDlgBox)
					{
						DestroyWindow(hCorrectDlgBox);
						hCorrectDlgBox = NULL;
					}
			}
			break;

		case IDM_REC_RECOGNIZE:
		case IDM_REC_UPPER:
			lpTrace = &(lpSect->trace);
			gFirstWordNum = FindSectNumber(lpHlv, lpSect);
			gCurSect = 0;
			if (gFirstWordNum >= 0)
			{
				tapRecognizeWord(hLastTAPWnd, gFirstWordNum, id == IDM_REC_UPPER);
			}
			else
			{
				// on line recognition
				gCurSect = -1;
				recRecognizeTrace(lpTrace, NULL, id == IDM_REC_UPPER, FALSE, 0);
			}
			break;

#ifdef _PENWIN
		case IDM_REC_CONFIG:
			recConfig();
			break;
#endif
	}
Error:
	DebugUnlockHandle(hHlv);
} /* end of hlvCommand */

/**************************************************************************/

int FAR hlvVfunction(HWND hWnd, WPARAM wParam)
{
	HWND     hInkWnd;
	HGLOBAL  hHlv;
	LP_HLV   lpHlv;
	int      OldLevel, OldSureLevel;
	LP_TRACE lpTrace;

	RETURN_IF_BAD_POINTER(hWnd);
	if (WaitFlag)
	{
		return FALSE;
	}
	if (wParam == IDM_V2_FUNCTION)
	{
		OldSureLevel = msgGetSureLevel();
		msgSetSureLevel(200);
	}
	OldLevel = LookLevel;
	LookLevel = 2;
	UpdateWindow(hWnd);
	hInkWnd = GetDlgItem(hWnd, DRAWINK);
	hHlv = (HGLOBAL) GetWindowLong(hInkWnd, GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);
	if (!lpHlv->fValid || lpHlv->head == NULL)
	{
		goto Error;
	}
	//
	//static int SaveEditedXRLine(LP_HLV lpHlv, LP_HLV_SECT lpSect)
	{
		int          k;
		LP_HLV_SECT  lpSect = lpHlv->lpCurSect;

		DISPOSE(lpNewXrLine, "WGHLV SaveEditedXRLine");
#ifdef _PENWIN
		lpNewXrLine = (p_XR)GlobalAllocPtr(GSHND, sizeof(XR_TYPE)*LAB_RW_SIZE*LAB_XR_SIZE);
#else
		lpNewXrLine = (p_LAB_XRDATA)
		              DebugAllocPtr(GSHND, sizeof(LAB_XRDATA_TYPE)*LAB_RW_SIZE*LAB_XR_SIZE,
		                            "WGHLV SaveEditedXRLine");
#endif
		RETURN_IF_BAD_POINTER(lpNewXrLine);
		bGetNewXrData = TRUE;
		k = 0;
#ifdef _PENWIN
		while(lpSect->xrs[k].xr != 0)
		{
			lpNewXrLine[k].xr       = lpSect->xrs[k].xr;
			lpNewXrLine[k].a        = lpSect->xrs[k].a;
			lpNewXrLine[k].p        = lpSect->xrs[k].p;
			lpNewXrLine[k].h        = lpSect->xrs[k].h;
			lpNewXrLine[k].begpoint = 0;
			lpNewXrLine[k].endpoint = 0;
			k++;
		}
#else
		while (lpSect->xrs[k].xdxr.xnxr != 0)
		{
			lpNewXrLine[k].xdxr.xnxr = lpSect->xrs[k].xdxr.xnxr;
			lpNewXrLine[k].xdxr.xna = lpSect->xrs[k].xdxr.xna;
			lpNewXrLine[k].xdxr.xnp = lpSect->xrs[k].xdxr.xnp;
			lpNewXrLine[k].xdxr.xnh = lpSect->xrs[k].xdxr.xnh;
			lpNewXrLine[k].xdbegpt = 0;
			lpNewXrLine[k].xdendpt = 0;
			k++;
		}
#endif
	} /* end of SaveEditedXRLine */
	lpTrace = &(lpHlv->lpCurSect->trace);
	gCurSect = FindSectNumber(lpHlv, lpHlv->lpCurSect);
	gFirstWordNum = 0;
	recRecognizeTrace(lpTrace, NULL, TRUE, FALSE, gFirstWordNum);
Error:
	DebugUnlockHandle(hHlv);
	LookLevel = OldLevel;
	msgUpdateLevels();
	if (wParam == IDM_V2_FUNCTION)
	{
		msgSetSureLevel(OldSureLevel);
	}
	return TRUE;
} /* end of Vfunction */

/**************************************************************************/
BOOL FAR hlvUpperDone(HWND hWnd)
{
	HGLOBAL  hHlv;
	LP_HLV   lpHlv;
	BOOL     result;

	RETURN_IF_BAD_POINTER(hWnd);
	if (WaitFlag)
	{
		return FALSE;
	}
	hHlv = (HGLOBAL) GetWindowLong(hWnd, GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);
	result = lpHlv->fUpperDone;
	DebugUnlockHandle(hHlv);
	return result;
} /* end of hlvUpperDone */

/**************************************************************************/

int FAR hlvResetSectionList(HWND hWnd, HWND hTAPWnd, BOOL InAnyCase)
{
	HGLOBAL  hHlv;
	LP_HLV   lpHlv;
	int      nWords, result;
	char     TAPname[_MAX_PATH];

	hHlv = (HGLOBAL) GetWindowLong(hWnd, GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);
	tapGetTapFileName(hLastTAPWnd, TAPname, &nWords);

	if (!InAnyCase && !lstrcmpi(TAPname, lpHlv->szTAPFileName))
	{
		// the same file - do nothing
		DebugUnlockHandle(hHlv);
		return TRUE;
	}
	TrashAll(lpHlv, TRUE);
	// Create new section list
	lstrcpy(lpHlv->szTAPFileName, TAPname);
	result = BuildSectionList(lpHlv, nWords);
	DebugUnlockHandle(hHlv);
	return result;
} /* end of hlvResetSectionList */

/**************************************************************************/

BOOL FAR hlvIsCurSectEmpty(HWND hWnd)
{
	HGLOBAL  hHlv;
	LP_HLV   lpHlv;
	BOOL     result;

	hHlv = (HGLOBAL) GetWindowLong(GetDlgItem(hWnd, DRAWINK), GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);
	result = (lpHlv->lpCurSect->sectionType == EMPTY_SECTION);
	DebugUnlockHandle(hHlv);
	return result;
} /* end of hlvIsCurSectEmpty  */

/*************************************************************************/
DWORD FAR hlvDrawXrLine(HDC hDC, LPRECT lprc, int SelectedChar)
{
	HGLOBAL       hHlv;
	LP_HLV        lpHlv;
	int           n;
	int           n_xrs;
	int           nXOff, nYOff, nFontSize;
	DWORD         wh;
	int           width;
	LP_HLV_SECT   lpSect;
#ifdef _PENWIN
	p_XR          xrs;
#else
	p_LAB_XRDATA  xrs;
#endif

	hHlv = (HGLOBAL) GetWindowLong(GetDlgItem(hLastDebugWnd, DRAWINK), GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);
	lpSect = lpHlv->lpCurSect;
	n_xrs = lpSect->n_xrs;
	xrs = lpSect->xrs;
	nFontSize = (lprc->bottom - lprc->top) / 2;
	nXOff = 0;
	nYOff = lprc->top;
	FillRect(hDC, lprc, (HBRUSH) GetStockObject(BLACK_BRUSH));
	width = 0; //for less warnings
#ifdef _PENWIN
	for(n = 0; n < n_xrs && xrs[n].xr != 0; n++)
	{
#else
	for (n = 0; n < n_xrs && xrs[n].xdxr.xnxr != 0; n++)
	{
#endif
		wh = xrdDraw(hDC, nXOff + n*width, nYOff, -nFontSize, (LPSTR) &(xrs[n]), 1, TRUE, NULL);
		width = (int) LOWORD(wh);
		if (SelectedChar == n)
		{
			SelectObject(hDC, GetStockObject(WHITE_PEN));
			MoveToEx(hDC, nXOff + n*width, lprc->top + 1, NULL);
			LineTo(hDC, nXOff + (n + 1)*width, lprc->top + 1);
			MoveToEx(hDC, nXOff + n*width, lprc->bottom - 1, NULL);
			LineTo(hDC, nXOff + (n + 1)*width, lprc->bottom - 1);
			SelectObject(hDC, GetStockObject(BLACK_PEN));
		}
	}
	DebugUnlockHandle(hHlv);
	n = 0;
#ifdef _PENWIN
	while(n < n_xrs && xrs[n].xr != 0)
	{
		n++;
	}
#else
	while (n < n_xrs && xrs[n].xdxr.xnxr != 0)
	{
		n++;
	}
#endif
	return (MAKELONG(width, n));
} /* end of hlvDrawXrLine */

/*************************************************************************/
DWORD FAR hlwGetXrExtent(LPRECT lprc, int CaretPos)
{
	int     nFontSize, Offset;

	nFontSize = (lprc->bottom - lprc->top) / 2;
	Offset = lprc->left + CaretPos*nFontSize * 2 / 3;
	return MAKELONG(Offset, nFontSize * 2 / 3);
} /* end of hlwGetXrExtent */

/*************************************************************************/
BOOL FAR hlvCreateStringCopy(HWND hWnd)
{
	HGLOBAL  hHlv;
	LP_HLV   lpHlv;

	lpEditSection = (LP_HLV_SECT) DebugAllocPtr(GSHND, sizeof(HLV_SECT_TYPE), "WGHLV hlvCreateStringCopy");
	hHlv = (HGLOBAL) GetWindowLong(GetDlgItem(hWnd, DRAWINK), GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);
	if (lpHlv->lpCurSect == NULL)
	{
		DebugUnlockHandle(hHlv);
		return FALSE;
	}
	_fmemcpy(lpEditSection, lpHlv->lpCurSect, sizeof(HLV_SECT_TYPE));
	DebugUnlockHandle(hHlv);
	return TRUE;
} /* end of hlvCreateStringCopy */

/*************************************************************************/
int  FAR hlvXREditDone(HWND hWnd, BOOL Save)
{
	HGLOBAL     hHlv;
	LP_HLV      lpHlv;
	HWND        hInkWnd;
	LP_HLV_SECT lpSect;

	RETURN_IF_BAD_POINTER(hWnd);
	hInkWnd = GetDlgItem(hWnd, DRAWINK);
	hHlv = (HGLOBAL) GetWindowLong(hInkWnd, GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);
	if (lpHlv->lpCurSect != NULL)
	{
		if (!Save)
		{
			_fmemcpy(lpHlv->lpCurSect, lpEditSection, sizeof(HLV_SECT_TYPE));
		}
		else
		{
			lpSect = lpHlv->lpCurSect;
			SaveEditedXRLine(lpHlv, lpSect);
		}
	}
	DebugUnlockHandle(hHlv);
	DISPOSE(lpEditSection, "WGHLV hlvXREditDone");
	return TRUE;
} /* end of hlvXREditDone */

/*************************************************************************/
static int SaveEditedXRLine(LP_HLV lpHlv, LP_HLV_SECT lpSect)
{
	LP_TRACE    lpTrace;
	int         k;

	DISPOSE(lpNewXrLine, "WGHLV SaveEditedXRLine");
#ifdef _PENWIN
	lpNewXrLine = (p_XR)DebugAllocPtr(GSHND, sizeof(XR_TYPE)*LAB_RW_SIZE*LAB_XR_SIZE,
	                                  "WGHLV SaveEditedXRLine");
#else
	lpNewXrLine = (p_LAB_XRDATA)
	              DebugAllocPtr(GSHND, sizeof(LAB_XRDATA_TYPE)*LAB_RW_SIZE*LAB_XR_SIZE,
	                            "WGHLV SaveEditedXRLine");
#endif
	RETURN_IF_BAD_POINTER(lpNewXrLine);
	bGetNewXrData = TRUE;
	k = 0;
	while (lpSect->xrs[k].xdxr.xnxr != 0)
	{
		lpNewXrLine[k].xdxr.xnxr = lpSect->xrs[k].xdxr.xnxr;
		lpNewXrLine[k].xdxr.xna = lpSect->xrs[k].xdxr.xna;
		lpNewXrLine[k].xdxr.xnp = lpSect->xrs[k].xdxr.xnp;
		lpNewXrLine[k].xdxr.xnh = lpSect->xrs[k].xdxr.xnh;
		lpNewXrLine[k].xdxr.xns = lpSect->xrs[k].xdxr.xns;
		lpNewXrLine[k].xdxr.xno = lpSect->xrs[k].xdxr.xno;
		lpNewXrLine[k].xdxr.xnd = lpSect->xrs[k].xdxr.xnd;
		lpNewXrLine[k].xdbegpt = lpSect->xrs[k].xdbegpt;
		lpNewXrLine[k].xdendpt = lpSect->xrs[k].xdendpt;
		lpNewXrLine[k].xdleft = lpSect->xrs[k].xdleft;
		lpNewXrLine[k].xdright = lpSect->xrs[k].xdright;
		lpNewXrLine[k].xdtop = lpSect->xrs[k].xdtop;
		lpNewXrLine[k].xdbottom = lpSect->xrs[k].xdbottom;
		k++;
	}
	lpTrace = &(lpSect->trace);
	gFirstWordNum = FindSectNumber(lpHlv, lpSect);
	gCurSect = 0;
	recRecognizeTrace(lpTrace, NULL, FALSE, FALSE, gFirstWordNum);
	return TRUE; //CHE 12-27-94
} /* end of SaveEditedXRLine */

/*************************************************************************/
int FAR hlvGetStringLength(void)
{
	HGLOBAL       hHlv;
	LP_HLV        lpHlv;
	int           n;
	int           n_xrs;
#ifdef _PENWIN
	p_XR          xrs;
#else
	p_LAB_XRDATA  xrs;
#endif
	LP_HLV_SECT   lpSect;

	hHlv = (HGLOBAL) GetWindowLong(
	           GetDlgItem(hLastDebugWnd, DRAWINK), GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);
	lpSect = lpHlv->lpCurSect;
	n_xrs = lpSect->n_xrs;
	xrs = lpSect->xrs;
#ifdef _PENWIN
	for (n = 0; n < n_xrs && xrs[n].xr != 0; n++);
#else
	for (n = 0; n < n_xrs && xrs[n].xdxr.xnxr != 0; n++);
#endif
	DebugUnlockHandle(hHlv);
	return (n);
} /* hlvGetStringLength */

/*************************************************************************/
int  FAR hlvEditString(LPSTR pXR, int Pos, WPARAM OpCode)
{
	HGLOBAL hHlv;
	LP_HLV  lpHlv;
	int     n_xrs, n, k;
#ifdef _PENWIN
	int           size = sizeof(XR_TYPE);
	p_XR          xrs;
#else
	int           size = sizeof(LAB_XRDATA_TYPE);
	p_LAB_XRDATA  xrs;
#endif
	LP_HLV_SECT lpSect;

	hHlv = (HGLOBAL) GetWindowLong(GetDlgItem(hLastDebugWnd, DRAWINK), GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);
	lpSect = lpHlv->lpCurSect;
	n_xrs = lpSect->n_xrs;
	xrs = lpSect->xrs;
#ifdef _PENWIN
	for (n = 0; n < n_xrs && xrs[n].xr != 0; n++);
#else
	for (n = 0; n < n_xrs && xrs[n].xdxr.xnxr != 0; n++);
#endif
	switch (OpCode)
	{
		case ID_REPLACE:
			_fmemcpy(&xrs[Pos], pXR, size);
			break;
		case ID_DELETE:
			_fmemcpy(&xrs[Pos], &xrs[Pos + 1], size*(n - Pos + 1));
			_fmemcpy(pXR, &xrs[Pos], size);
			break;
		case ID_INSERT:
			for (k = n - 1; k >= Pos; k--)
			{
				_fmemcpy(&xrs[k + 1], &xrs[k], size);
			}
			_fmemcpy(&xrs[Pos], pXR, size);
			// copy new selection
			_fmemcpy(pXR, &xrs[Pos + 1], size);
			break;
	}
	DebugUnlockHandle(hHlv);
	return 0;
} /* hlvEditString */

/***********************************************************************/
void FAR hlvGetXRitem(LPSTR pXR, int index)
{
	HGLOBAL hHlv;
	LP_HLV  lpHlv;
#ifdef _PENWIN
	int           size = sizeof(XR_TYPE);
	p_XR          xrs;
#else
	int           size = sizeof(LAB_XRDATA_TYPE);
	p_LAB_XRDATA  xrs;
#endif

	hHlv = (HGLOBAL) GetWindowLong(GetDlgItem(hLastDebugWnd, DRAWINK), GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);
	xrs = lpHlv->lpCurSect->xrs;
	_fmemcpy(pXR, &xrs[index], size);
	DebugUnlockHandle(hHlv);
} /* hlvGetXRitem */

/**********************************************************************/
LPSTR FAR hlvGetTrace(HWND hWnd)
{
	HGLOBAL     hHlv;
	LP_HLV      lpHlv;
	LP_TRACE    lpTrace;

	hHlv = (HGLOBAL) GetWindowLong(hWnd, GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);
	lpTrace = NULL;
	if (lpHlv->lpCurSect != NULL)
	{
		lpTrace = (LP_TRACE) &lpHlv->lpCurSect->trace;
	}
	DebugUnlockHandle(hHlv);
	return (LPSTR) lpTrace;
} /* hlvGetTrace */

/*********************************************************************/
void FAR hlvDeleteOnlineSections()
{
	HGLOBAL hHlv;
	LP_HLV  lpHlv;

	if (hLastDebugWnd)
	{
		hHlv = (HGLOBAL) GetWindowLong(GetDlgItem(hLastDebugWnd, DRAWINK), GWW_HLV_DATA);
		lpHlv = (LP_HLV) DebugLockHandle(hHlv);
		// kill only not TAP sections
		TrashAll(lpHlv, FALSE);
		DebugUnlockHandle(hHlv);
	}
} /* hlvDeleteOnlineSections */

/*****************************************************************************/
BOOL FAR hlvSectionListEmpty(HWND hInkWnd)
{
	HGLOBAL     hHlv;
	LP_HLV      lpHlv;
	BOOL        Empty;

	hHlv = (HGLOBAL) GetWindowLong(hInkWnd, GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);
	// check if no sections at all or only automatic ones
	// rebuild section list in this case
	Empty = (lpHlv->head == NULL || lpHlv->head->index < 0);
	DebugUnlockHandle(hHlv);
	return Empty;
} /* end of hlvSectionListEmpty */

/*****************************************************************************/
#ifdef _PENWIN
void FAR hlvCheckWordCut(HWND hWnd, HPENDATA hpendata)
{
	BOOL        BeginWord;
	int         PrevSect;
	HGLOBAL     hHlv;
	LP_HLV      lpHlv;
	LP_HLV_SECT lpSect;

	hHlv = (HGLOBAL) GetWindowLong(hWnd, GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);

	lpSect = lpHlv->lpCurSect;
	if (lpSect == NULL)
	{
		DebugUnlockHandle(hHlv);
		MessageBeep(0);
		return;
	}
	if (gCurSect < 0 )
	{
		// auto section
		lpSect->infoType = -1;
		DebugUnlockHandle(hHlv);
		return;
	}
	PrevSect = gCurSect;

	lpSect->infoType = tapSetNewSaveParam(lpSect->trace.nStrokes, &gCurSect,
	                                      &gInsertBranchSection, &BeginWord);

	UpdateSectNumber(lpSect);
	//??SD 11.09.93
	if (lpSect->PassNum == 2)
	{
		lpSect->prev->infoType = lpSect->infoType;
	}
	if (BeginWord)   // kill all the branch sections of gCurSect
	{
		KillBranchSections(hWnd, PrevSect + gFirstWordNum);
	}
	DebugUnlockHandle(hHlv);
} /* end of hlvCheckWordCut */
#endif
/*****************************************************************************/

static void KillBranchSections(HWND hWnd, int SectNum)
{
	HGLOBAL     hHlv;
	LP_HLV      lpHlv;
	LP_HLV_SECT lpSect, lpTemp;

	if (hWnd)
	{
		hHlv = (HGLOBAL) GetWindowLong(hWnd, GWW_HLV_DATA);
		lpHlv = (LP_HLV) DebugLockHandle(hHlv);
		lpSect = FindSectByNumber(lpHlv, SectNum);
		//??SD 09.08.93
		// kill all branch sections but the first, if it is of 2-nd pass
		if (lpSect == NULL)
		{
			goto out;
		}
		if (lpSect->next != NULL && lpSect->next->index == SectNum &&
		        lpSect->next->PassNum == 2)
		{
			lpSect = lpSect->next;
		}
		while (lpSect->next != NULL && lpSect->next->index == SectNum)
		{
			lpTemp = lpSect->next;
			lpSect->next = lpSect->next->next;
			if (lpSect->next != NULL)
			{
				lpSect->next->prev = lpSect;
			}
			else
			{
				lpHlv->tail = lpSect;
			}
			TrashSect(lpTemp);
			DebugFreePtr(lpTemp, "WGHLV KillBranchSections");
		}
out:
		DebugUnlockHandle(hHlv);
	}
} /* end of KillBranchSections */

/*****************************************************************************/
static void UpdateSectNumber(LP_HLV_SECT lpSect)
{
	char buff[MAX_SECT_TEXT];
	char pass[19] = "";

	if (lpSect->PassNum == 2)
	{
		wsprintf(pass, "%dnd pass", lpSect->PassNum);
	}
	switch (lpSect->infoType)
	{
		case BOUNDARY:
			wsprintf(buff, "%d %s", lpSect->index + 1, pass);
			break;
		case CUT:
			wsprintf(buff, "%d %s %s", lpSect->index + 1, "cut", pass);
			break;
		case GLUED:
			wsprintf(buff, "%d %s %s", lpSect->index + 1, "glued", pass);
			break;
		case EMPTY_SECT:
			lstrcpy(buff, "");
			break;
		default:
			wsprintf(buff, "%s %s", "on-line", pass);
	}
	tlsSetSectNum(buff);
} /* end of UpdateSEctNumber */

/*******************************************************************************/
static void DrawGraph(HDC hDC, LP_HLV_SECT lpSect, LPRECT lprc, HFONT hFont)
{
	char                  text[3] = "";
	int                   i, nXOff, nYOff;
	p_GRAPH_TYPE          graph;
	TEXTMETRIC            tm;
	RECT                  rc;

	/**
	  if (!lpSect->graph)
	  return;
	  **/
	GetTextMetrics(hDC, &tm);
	lpSect->chh = tm.tmHeight;
	lpSect->chw = tm.tmMaxCharWidth;
	graph = &lpSect->graph;
	i = 0;
	while (i < (int) graph->size)
	{
		if (GRTYPE(graph->prwsb, i) == LAB_RWST_SYM)
		{
			SetTextColor(hDC, GetGraphColor(GRWEIGHT(graph->prwsb, i)));
			text[0] = GRSYMBOL(graph->prwsb, i);
			text[1] = 0;
			if ((i + 1) < (_SHORT) graph->size && GRTYPE(graph->prwsb, i + 1) == LAB_RWST_SYM &&
			        GRUSER(graph->prwsb, i + 1) == GRUSER(graph->prwsb, i))
			{
				text[1] = GRSYMBOL(graph->prwsb, i + 1);
				text[2] = 0;
			}
			nXOff = lprc->left + HIWORD(graph->pnumb[i])*(2 * lpSect->chw + 2) + LEFT_MARGIN;
			nYOff = lprc->top + LOWORD(graph->pnumb[i])*lpSect->chh +
			        TOP_MARGIN + lpSect->chh;
			TextOut(hDC, nXOff, nYOff, text, lstrlen(text));
			if (graph->pselb[i])
			{
				DrawSelectedCharWeights(hDC, graph, lprc, lpSect->chw, i);
				GetSymbolRect(graph, lprc, i, lpSect->chw, lpSect->chh, &rc);
				DrawCharSelRect(hDC, &rc, WHITE_PEN);
			}
			if ((i + 1) < (_SHORT) graph->size && GRTYPE(graph->prwsb, i + 1) == LAB_RWST_SYM &&
			        GRUSER(graph->prwsb, i + 1) == GRUSER(graph->prwsb, i))
			{
				i++;
			}
		}
		i++;
	}
} /* end of DrawGraph */

/*******************************************************************************/
static void DrawSelectedCharWeights(HDC hDC, p_GRAPH_TYPE graph,
                                    LPRECT lprc, int ch_width, int i)
{

#define WEIGHTS "Wgt: "
#define PPWEIGHTS "pp-> "
	int     offset;
	char    weight[6];
	SIZE    size;
#ifdef _PENWIN
	unsigned char nexttype = graph->prwsb[i+1].type;
	unsigned char user     = graph->prwsb[i].d_user;
	unsigned char nextuser = graph->prwsb[i+1].d_user;
	unsigned char cweight   = graph->prwse[i].weight;
	unsigned char nextweight= graph->prwse[i+1].weight;
#else
	unsigned char nexttype = graph->prwsb[i + 1].rntype;
	unsigned char user = graph->prwsb[i].rnuser;
	unsigned char nextuser = graph->prwsb[i + 1].rnuser;
	unsigned char cweight = graph->prwse[i].rnweight;
	unsigned char nextweight = graph->prwse[i + 1].rnweight;
#endif

	SetBkColor(hDC, RGB(0, 0, 0));
	offset = lprc->left + LEFT_MARGIN;
	SetTextColor(hDC, RGB(255, 255, 255));
	TextOut(hDC, offset, lprc->top + TOP_MARGIN, WEIGHTS, lstrlen(WEIGHTS));

	GetTextExtentPoint(hDC, WEIGHTS, lstrlen(WEIGHTS), &size);
	offset += size.cx;
	SetTextColor(hDC, GetGraphColor(cweight));
	_itoa(cweight, weight, 10);
	TextOut(hDC, offset, lprc->top + TOP_MARGIN, weight, lstrlen(weight));
	GetTextExtentPoint(hDC, weight, lstrlen(weight), &size);
	offset += size.cx;
	TextOut(hDC, offset, lprc->top + TOP_MARGIN, "        ", 4);
	offset += ch_width;

	SetTextColor(hDC, RGB(255, 255, 255));
	TextOut(hDC, offset, lprc->top + TOP_MARGIN, PPWEIGHTS, lstrlen(PPWEIGHTS));
	GetTextExtentPoint(hDC, PPWEIGHTS, lstrlen(PPWEIGHTS), &size);
	offset += size.cx;

	SetTextColor(hDC, GetGraphColor(cweight));
	_itoa(cweight, weight, 10);
	TextOut(hDC, offset, lprc->top + TOP_MARGIN, weight, lstrlen(weight));
	GetTextExtentPoint(hDC, weight, lstrlen(weight), &size);
	offset += size.cx;
	TextOut(hDC, offset, lprc->top + TOP_MARGIN, "          ", 6);

	if (i < graph->size - 1 && nexttype == LAB_RWST_SYM && user == nextuser)
	{
		GetTextExtentPoint(hDC, weight, lstrlen(weight), &size);
		offset += size.cx;
		TextOut(hDC, offset, lprc->top + TOP_MARGIN, "  ", 2);
		offset += ch_width;
		SetTextColor(hDC, GetGraphColor(nextweight));
		_itoa(nextweight, weight, 10);
		TextOut(hDC, offset, lprc->top + TOP_MARGIN, weight, lstrlen(weight));
	}
} /* end of DrawSelectedCharWeights  */

/*******************************************************************************/
COLORREF GetGraphColor(_UCHAR weight)
{
	if (weight == 100)
	{
		return RGB(255, 255, 255);
	}
	else
		if (weight < 100 && weight > 85)
		{
			return RGB(0, 128, 255);
		}
		else
			if (weight <= 85 && weight > 50)
			{
				return RGB(128, 255, 0);
			}
			else
			{
				return RGB(255, 0, 0);
			}
} /* end of GetGraphColor */

/*******************************************************************************/
static void GetSymbolRect(p_GRAPH_TYPE graph, LPRECT rcWnd,
                          int index, int width, int height, LPRECT rcSym)
{
#define GRCOL(x, i) HIWORD(x[i])
#define GRROW(x, i) LOWORD(x[i])
	SetRect(rcSym, 0, 0, 0, 0);
	if (GRTYPE(graph->prwsb, index) == LAB_RWST_SYM)
	{
		rcSym->left = rcWnd->left + LEFT_MARGIN + GRCOL(graph->pnumb, index)*(2 * width + 2);
		rcSym->top = rcWnd->top + TOP_MARGIN + height + GRROW(graph->pnumb, index)*height;
		rcSym->right = rcSym->left + width;
		rcSym->bottom = rcSym->top + height;
		if ((UINT) (index + 1) < graph->size && GRTYPE(graph->prwsb, index + 1) == LAB_RWST_SYM &&
		        GRUSER(graph->prwsb, index + 1) == GRUSER(graph->prwsb, index))
		{
			rcSym->right += width;
		}
	}
} /* end of GetSymbolRect */

/*******************************************************************************/
static BOOL SelectNewChar(HWND hWnd, LP_HLV_SECT lpSect, LP_HLV lpHlv, LPARAM lParam,
                          int SelectedChar)
{
	p_GRAPH_TYPE     graph;
	POINT             pt;
	_SHORT            i, k;
	BOOL              result;
	RECT              rc, rc1;
	HDC               hDC;
	LOGFONT           lf;
	HFONT             hFont = NULL, hOldFont;

	_fmemset(&lf, 0, sizeof(LOGFONT));
	lf.lfHeight = -(lpHlv->rcGraph.bottom - lpHlv->rcGraph.top - 2) / (LAB_XR_SIZE + 2);
	lstrcpy(lf.lfFaceName, "Courier New");
	hFont = CreateFontIndirect(&lf);

	graph = &lpSect->graph;
	hDC = GetDC(hWnd);
	hOldFont = (HFONT) SelectObject(hDC, hFont);
	if (!SelectedChar)
	{
		pt.x = LOWORD(lParam);
		pt.y = HIWORD(lParam);
	}
	result = FALSE;
	for (i = (SelectedChar) ? (SelectedChar) : (0); i < (_SHORT) graph->size; i++)
	{
		GetSymbolRect(graph, &lpHlv->rcGraph, i, lpSect->chw, lpSect->chh, &rc);
		if (PtInRect(&rc, pt) || (SelectedChar != 0 && SelectedChar == i))
		{
			// kill old selection
			for (k = 0; k < (_SHORT) graph->size; k++)
			{
				if (graph->pselb[k])
				{
					GetSymbolRect(graph, &lpHlv->rcGraph, k, lpSect->chw, lpSect->chh, &rc1);
					DrawCharSelRect(hDC, &rc1, BLACK_PEN);
					graph->pselb[k] = 0;
					break;
				}
			}
			// set  new selection
			graph->pselb[i] = 1;
			DrawCharSelRect(hDC, &rc, WHITE_PEN);
			DrawSelectedCharWeights(hDC, graph, &lpHlv->rcGraph, lpSect->chw, i);
			result = TRUE;
			InvalidateRect(hWnd, &lpHlv->rcInk, TRUE);
			InvalidateRect(hWnd, &lpHlv->rcXrs, TRUE);
			UpdateWindow(hWnd);
			break;
		}
	}
	if (hFont)
	{
		SelectObject(hDC, hOldFont);
		DeleteObject(hFont);
	}
	ReleaseDC(hWnd, hDC);
	return result;
} /* end of SelectNewChar */

/*******************************************************************************/
static void DrawCharSelRect(HDC hDC, LPRECT rc, int Color)
{
	HPEN hPenOld;
	hPenOld = (HPEN) SelectObject(hDC, GetStockObject(Color));
	MoveToEx(hDC, rc->left, rc->top, NULL);
	LineTo(hDC, rc->left, rc->bottom);
	LineTo(hDC, rc->right, rc->bottom);
	LineTo(hDC, rc->right, rc->top);
	LineTo(hDC, rc->left, rc->top);
	SelectObject(hDC, hPenOld);
} /* end of DrawCharSelRect */

/********************************************************************************/
static p_PPD SetRWPPDData(LP_HLV_SECT lpSect, int sel_word, DWORD *Word)
{
	UINT                i, j, word_index, length;
	char                AnswerWord[LAB_RW_SIZE], GraphWord[LAB_RW_SIZE + 1];
	p_PPD               ppd = NULL;
	p_GRAPH_TYPE        graph;
#ifdef _PENWIN
	p_RWG_PPD_el_type ppd_el_sel;
#else
	p_LAB_PPNODE      ppd_el_sel;
#endif


	RETURN_IF_BAD_POINTER(lpSect->answers);
	graph = &lpSect->graph;
	RETURN_IF_BAD_POINTER(graph);
	RETURN_IF_BAD_POINTER(graph->prwsb);
	RETURN_IF_BAD_POINTER(graph->pppdb);
	lstrcpy(AnswerWord, lpSect->answers[sel_word].anword);

	//??SD it happens that graph->prwse and graph->prwsb are in different code mode
	//     prwsb - MAC encoding
	//     prwse - Recognizer encoding
	//     as lpSect->answers[i].anword are in MAC encoding
	//        we search word using  graph->prwsb
	word_index = 0;
	i = 0;
	j = 0;
	while (i < graph->size)
	{
		if (GRTYPE(graph->prwsb, i) == LAB_RWST_SPLIT)
		{
			word_index = i + 1;
			j = 0;
		}
		else
			if (GRTYPE(graph->prwsb, i) == LAB_RWST_NEXT ||
			        GRTYPE(graph->prwsb, i) == LAB_RWST_JOIN)
			{
				GraphWord[j] = 0;
				if (lstrcmp(GraphWord, AnswerWord) == 0)
				{
					break;
				}
				j = 0;
				GraphWord[j] = 0;
				word_index = i + 1;
			}
			else
				if (GRTYPE(graph->prwsb, i) == LAB_RWST_SYM)
				{
					GraphWord[j] = GRSYMBOL(graph->prwsb, i);
					GraphWord[++j] = 0;
				}
		i++;
	}
	if (i == graph->size)
	{
		if (lstrcmp(GraphWord, AnswerWord) != 0)
		{
			return ppd;
		}
	}
	length = lstrlen(GraphWord);
	if (Word != NULL)
	{
		// return back into DrawTrace procedure - no need for ppd
		*Word = (DWORD) MAKELONG(word_index, length);
		return ppd;
	}
	ppd = (p_PPD) DebugAllocPtr(GHND, (length + 1)*sizeof(PPD_TYPE), "WGHLV SetRWPPDData");
	if (!ppd)
	{
		return ppd;
	}
	// word_index points to the word beginning in graph
	//??SD
	{
		int l = 0;
		for (i = 0; i < length; i++)
		{
			j = word_index + i;
			ppd_el_sel = (p_LAB_PPNODE) ((LPSTR) graph->pppdb + j*sizeof(LAB_RWG_PPD_ARRAY));
			if (GRSYMBOL(graph->prwsb, j) != ' ') /* GIT */
			{
				FillPPDElement(&ppd[l], (LPSTR) ppd_el_sel,
				               GRSYMBOL(graph->prwsb, j),
				               GRRSYMBOL(graph->prwsb, j),
				               GRNVAR(graph->prwsb, j),
				               GRWEIGHT(graph->prwsb, j));
				l++;
			}
			ppd[l].letter = 0;
		}
	}
	return ppd;
} /* end of SetRWPPDData */

/********************************************************************************/
static p_PPD SetGraphPPDData(LP_HLV_SECT lpSect)
{
	p_PPD               ppd = NULL;
	p_GRAPH_TYPE        graph;
#ifdef _PENWIN
	p_RWG_PPD_el_type ppd_el, ppd_el_sel;
#else
	p_LAB_PPNODE      ppd_el, ppd_el_sel;
#endif
	int                 i, j, insert_sel, d_user;
	BOOL                CharPair = FALSE;

	graph = &lpSect->graph;
	if (!graph)
	{
		return (p_PPD) graph;
	}
	ppd = (p_PPD) DebugAllocPtr(GHND, (graph->size + 1)*sizeof(PPD_TYPE), "WGHLV SetGraphPPDData");
	// pointer is to be freed in DrawXr procedure
	if (!ppd)
	{
		return ppd;
	}
	ppd_el = graph->pppdb;
	d_user = -1;
	j = 0;
	i = 0;
	insert_sel = 0;
	while (i < (int) graph->size)
	{
		if (GRTYPE(graph->prwsb, i) == LAB_RWST_SYM && GRUSER(graph->prwsb, i) != d_user)
		{
			d_user = GRUSER(graph->prwsb, i);
			// new column beginning
#ifdef PENWIN
			ppd_el = (p_RWG_PPD_el_type)((LPSTR)graph->pppdb + i*sizeof(RWG_PPD_type));
#else
			ppd_el = (p_LAB_PPNODE) ((LPSTR) graph->pppdb + i*sizeof(LAB_RWG_PPD_ARRAY));
#endif
			// write down first symbol in column
			FillPPDElement(&ppd[j], (LPSTR) ppd_el,
			               GRSYMBOL(graph->prwsb, i),
			               GRRSYMBOL(graph->prwsb, i),
			               GRNVAR(graph->prwsb, i),
			               GRWEIGHT(graph->prwsb, i));
			ppd[j].xr[0] = 0;
			insert_sel = j;
			j++;
			if ((i + 1) < (_SHORT) graph->size && GRTYPE(graph->prwsb, i + 1) == LAB_RWST_SYM &&
			        GRUSER(graph->prwsb, i + 1) == d_user)
			{
				// first symbol in column is "glued" - write down second
				CharPair = TRUE;
#ifdef PENWIN
				ppd_el = (p_RWG_PPD_el_type)((LPSTR)graph->pppdb + (i+1)*sizeof(RWG_PPD_type));
#else
				ppd_el = (p_LAB_PPNODE) ((LPSTR) graph->pppdb + (i + 1)*sizeof(LAB_RWG_PPD_ARRAY));
#endif
				FillPPDElement(&ppd[j], (LPSTR) ppd_el,
				               GRSYMBOL(graph->prwsb, i + 1),
				               GRRSYMBOL(graph->prwsb, i + 1),
				               GRNVAR(graph->prwsb, i + 1),
				               GRWEIGHT(graph->prwsb, i + 1));
				ppd[j].xr[0] = 0;
				j++;
			}
			while (i < (_SHORT) graph->size)
			{
				if (GRTYPE(graph->prwsb, i) != LAB_RWST_SYM)
				{
					goto next1;
				}
				else
					if (d_user != GRUSER(graph->prwsb, i))
					{
						// next column begin
						i--;
						goto next;
					}
				// look through column, if it contains selected char
				if (graph->pselb[i] != 0)
				{
					// yes!!
					// insert selected symbol into "insert_sel" position
#ifdef PENWIN
					ppd_el_sel = (p_RWG_PPD_el_type)((LPSTR)graph->pppdb + i*sizeof(RWG_PPD_type));
#else
					ppd_el_sel = (p_LAB_PPNODE) ((LPSTR) graph->pppdb + i*sizeof(LAB_RWG_PPD_ARRAY));
#endif
					if (GRTYPE(graph->prwsb, i) == LAB_RWST_SYM)
					{
						FillPPDElement(&ppd[insert_sel], (LPSTR) ppd_el_sel,
						               GRSYMBOL(graph->prwsb, i),
						               GRRSYMBOL(graph->prwsb, i),
						               GRNVAR(graph->prwsb, i),
						               GRWEIGHT(graph->prwsb, i));
						ppd[insert_sel + 1].letter = 0;
						if (CharPair)
						{
							j--;
						}
						if ((i + 1) < (_SHORT) graph->size && GRTYPE(graph->prwsb, i + 1) == LAB_RWST_SYM &&
						        GRUSER(graph->prwsb, i + 1) == d_user)
						{
							// selected char is also "glued"
#ifdef PENWIN
							ppd_el_sel = (p_RWG_PPD_el_type)((LPSTR)graph->pppdb + (i+1)*sizeof(RWG_PPD_type));
#else
							ppd_el_sel = (p_LAB_PPNODE) ((LPSTR) graph->pppdb + (i + 1)*sizeof(LAB_RWG_PPD_ARRAY));
#endif
							FillPPDElement(&ppd[insert_sel + 1], (LPSTR) ppd_el_sel,
							               GRSYMBOL(graph->prwsb, i + 1),
							               GRRSYMBOL(graph->prwsb, i + 1),
							               GRNVAR(graph->prwsb, i + 1),
							               GRWEIGHT(graph->prwsb, i + 1));
							j++;
						}
					}
					break;
				}  // end of if (selected character)
next1:
				i++;
			} // end of while
			CharPair = FALSE;
		}
next:
		CharPair = FALSE;
		i++;
	}
	ppd[j].letter = '\0';
	return ppd;
} /* end of SetGraphPPDData */

/********************************************************************************/
static void FillPPDElement(p_PPD Dest, LPSTR pSource,
                           char symb, char rsymb,
                           int variant, int percent)
{
	int       k;
#ifdef _PENWIN
	p_RWG_PPD_el_type Source = (p_RWG_PPD_el_type)pSource;
#else
	p_LAB_PPNODE      Source = (p_LAB_PPNODE) pSource;
#endif
	Dest->letter = symb;
	Dest->rletter = rsymb;
	Dest->percent = percent;
	Dest->num_var = variant;
	k = 0;
#ifdef _PENWIN
	while(Source[k].xr.xr != 0)
	{
		Dest->xr[k]    = Source[k].xr.xr;
		Dest->attr[k]  = Source[k].xr.a;
		Dest->height[k]= Source[k].xr.sh & 0xF;
		Dest->alias[k] = Source[k].alias;
		k++;
	}
	Dest->xr[k] = 0;
#else
	while (Source[k].pnxr.xnxr != 0)
	{
#ifdef VERTIGO
		Dest->xr[k]    = Source[k].pnxr.xnxr+(190-32);
#else
		Dest->xr[k] = Source[k].pnxr.xnxr;
#endif
		Dest->attr[k] = Source[k].pnxr.xna;
		Dest->height[k] = Source[k].pnxr.xnh & 0xF;
		Dest->alias[k] = Source[k].pnalias;
		k++;
	}
	Dest->xr[k] = 0;
#endif
} /* end of FillPPDElement */

/********************************************************************************/
#ifdef _PENWIN
void FAR hlvSetReadXRDataFlag(LPARAM lParam)
{
	bGetXRData = TRUE;
} /* end of hlvSetReadHLDataFlag */
#endif
/********************************************************************************/
static int GetNextGraphSymbol(p_GRAPH_TYPE graph, int j)
{
	int i;

	i = j + 1;
	if (i == (int) graph->size)
	{
		return 1;
	}
	if (GRTYPE(graph->prwsb, i) == LAB_RWST_SYM &&
	        GRUSER(graph->prwsb, i) == GRUSER(graph->prwsb, i - 1))
	{
		i++;
	}
	while (i < (int) graph->size && GRTYPE(graph->prwsb, i) != LAB_RWST_SYM)
	{
		i++;
	}
	return (i == (int) graph->size) ? (1) : (i);
} /* end of GetNextGraphSymbol */

/********************************************************************************/
static int GetPrevGraphSymbol(p_GRAPH_TYPE graph, int j)
{
	int i;

	i = j - 1;
	if (i < 0)
	{
		i = graph->size - 1;
	}
	while (i >= 0 && GRTYPE(graph->prwsb, i) != LAB_RWST_SYM)
	{
		i--;
	}
	if (i > 0)
		return (GRTYPE(graph->prwsb, i - 1) == LAB_RWST_SYM &&
		        GRUSER(graph->prwsb, i) == GRUSER(graph->prwsb, i - 1)) ? (i - 1) : (i);
	else
	{
		return 0;
	}
} /* end of GetPrevGraphSymbol */

/********************************************************************************/
static int GetLeftGraphSymbol(p_GRAPH_TYPE graph, int j, int row, int col)
{
	int i;

	i = j;
	if (col == 0)
	{
		// first column, find last char in the last column
		i = graph->size - 1;
		while (GRTYPE(graph->prwsb, i) != LAB_RWST_SYM)
		{
			i--;
		}
		if (i > 0 && GRTYPE(graph->prwsb, i - 1) == LAB_RWST_SYM &&
		        GRUSER(graph->prwsb, i) == GRUSER(graph->prwsb, i - 1))
			// "glued" characters, select first of them
		{
			i -= 1;
		}
	}
	else
	{
		// find last symbol of previous column
		i = j;
		while (TRUE)
		{
			if (GRTYPE(graph->prwsb, i) == LAB_RWST_SYM && (int) GRCOL(graph->pnumb, i) == col - 1)
			{
				break;
			}
			i--;
		}
		if (i > 0 && GRTYPE(graph->prwsb, i - 1) == LAB_RWST_SYM &&
		        GRUSER(graph->prwsb, i) == GRUSER(graph->prwsb, i - 1))
			// "glued" characters, select first of them
		{
			i -= 1;
		}
	}
	if ((int) GRROW(graph->pnumb, i) > row)
	{
		while (TRUE)
		{
			if (GRTYPE(graph->prwsb, i) == LAB_RWST_SYM && (int) GRROW(graph->pnumb, i) == row)
			{
				break;
			}
			i--;
		}
		if (i > 0 && GRTYPE(graph->prwsb, i - 1) == LAB_RWST_SYM &&
		        GRUSER(graph->prwsb, i) == GRUSER(graph->prwsb, i - 1))
			// "glued" characters, select first of them
		{
			i -= 1;
		}
	}
	return i;
} /* end of GetLeftGraphSymbol */

/********************************************************************************/
static int  GetRightGraphSymbol(p_GRAPH_TYPE graph, int j, int row, int col)
{
	int i;

	i = j;
	while (i < (int) graph->size)
	{
		// find first char on the next column
		if (GRTYPE(graph->prwsb, i) == LAB_RWST_SYM && (int) GRCOL(graph->pnumb, i) == col + 1)
		{
			break;
		}
		i++;
	}
	if (i == (int) graph->size)
	{
		// find first char on the next column
		i = 0;
		while (TRUE)
		{
			if (GRTYPE(graph->prwsb, i) == LAB_RWST_SYM)
			{
				break;
			}
			i++;
		}
	}
	if ((int) GRROW(graph->pnumb, i) < row)
	{
		while (TRUE)
		{
			if (i == (int) graph->size || GRTYPE(graph->prwsb, i) == LAB_RWST_SPLIT ||
			        GRTYPE(graph->prwsb, i) == LAB_RWST_JOIN)
			{
				// next column, select last char
				i = (i == (int) graph->size) ? (i - 1) : (i);
				while (TRUE)
				{
					if (GRTYPE(graph->prwsb, i) == LAB_RWST_SYM)
					{
						break;
					}
					i--;
				}
				if (i > 0 && GRTYPE(graph->prwsb, i - 1) == LAB_RWST_SYM &&
				        GRUSER(graph->prwsb, i) == GRTYPE(graph->prwsb, i - 1))
					// "glued" characters, select first of them
				{
					i--;
				}
				break;
			}
			else
				if (GRTYPE(graph->prwsb, i) == LAB_RWST_SYM &&
				        (int) GRROW(graph->pnumb, i) == row)
				{
					break;
				}
			i++;
		}
	}
	return i;
} /* end of GetRightGraphSymbol */

/********************************************************************************/
static int GetSelectedChar(p_GRAPH_TYPE graph, WORD Arrow)
{
	int     i, j, k;

	j = 0;
	while (graph->pselb[j] == 0)
	{
		j++;
	}
	i = (int) GRROW(graph->pnumb, j);
	k = (int) GRCOL(graph->pnumb, j);
	switch (Arrow)
	{
		case VK_DOWN:
			// select next character in graph
			j = GetNextGraphSymbol(graph, j);
			break;

		case VK_UP:
			// select previous character in the graph
			j = GetPrevGraphSymbol(graph, j);
			break;

		case VK_LEFT:
			// select last character in the prev. column in the graph
			j = GetLeftGraphSymbol(graph, j, i, k);
			break;

		case VK_RIGHT:
			// select last character in the next column in the graph
			j = GetRightGraphSymbol(graph, j, i, k);
			break;
	}
	return j;
} /* end of GetSelectedChar */

/********************************************************************************/
void FAR hlvUpdateScreen(HWND hWnd)
{
	if (hLastDebugWnd)
	{
		InvalidateRect(GetDlgItem(hLastDebugWnd, DRAWINK), NULL, TRUE);
	}
} /* end of hlvUpdateScreen */

/*******************************************************************************/
static void DrawSections(HDC hDC, LP_HLV lpHlv, LPRECT lprc)
{
	int         xMain, yMain;
	LP_HLV_SECT lpSect;

	if (lpHlv->head == NULL)
	{
		return;
	}
	yMain = lprc->top;
	xMain = lprc->left;
	lpSect = lpHlv->head;
	while (lpSect)
	{
		DrawSect(hDC, lpSect, &xMain, &yMain, lprc->top);
		lpSect = lpSect->next;
	}
} /* end of DrawSections */

static void DrawSect(HDC hDC, LP_HLV_SECT lpSect, int *x, int *y, int InitY)
{
#define       MaxLength 6
#define       Next      4
	TEXTMETRIC    tm;
	char          type[MaxLength];
	char          pass[MaxLength];
	char          sect[MaxLength];
	int           width, height;
	HPEN          hOldPen;
	HFONT         hOldFont;

	hOldPen = (HPEN) SelectObject(hDC, GetStockObject(WHITE_PEN));
	hOldFont = (HFONT) SelectObject(hDC, GetStockObject(ANSI_FIXED_FONT));
	GetTextMetrics(hDC, &tm);
	width = MaxLength*(tm.tmMaxCharWidth/*+2*/);
	height = 4 * (tm.tmHeight/*+2*/);
	MoveToEx(hDC, *x, *y, NULL);
	LineTo(hDC, *x, *y + height);
	LineTo(hDC, *x + width, *y + height);
	LineTo(hDC, *x + width, *y);
	LineTo(hDC, *x, *y);
	wsprintf(sect, "%d", (lpSect->index == -1) ? (-1) : (lpSect->index + 1));
	wsprintf(pass, "p-%d", lpSect->PassNum);
	switch (lpSect->infoType)
	{
		case BOUNDARY:
			wsprintf(type, "%s", "bndry");
			break;
		case CUT:
			wsprintf(type, "%s", "cut");
			break;
		case GLUED:
			wsprintf(type, "%s", "glued");
			break;
		case EMPTY_SECT:
			lstrcpy(type, "");
			break;
		default:
			wsprintf(type, "%s", "auto");
	}
	TextOut(hDC, *x + 2, *y + 2, sect, lstrlen(sect));
	TextOut(hDC, *x + 2, *y + 2 + tm.tmHeight, pass, lstrlen(pass));
	TextOut(hDC, *x + 2, *y + 2 + 2 * tm.tmHeight, type, lstrlen(type));
	if (lpSect->next != NULL)
	{
		if (lpSect->next->index != lpSect->index ||
		        (lpSect->next->index == -1 && lpSect->next->PassNum != 2))
		{
			*y = InitY;
			*x = *x + width + Next;
			MoveToEx(hDC, *x, *y + height / 2, NULL);
			LineTo(hDC, *x - Next, *y + height / 2);
		}
		else
		{
			MoveToEx(hDC, *x + width / 2, *y + height, NULL);
			*y += height + Next;
			LineTo(hDC, *x + width / 2, *y);
		}
	}
	SelectObject(hDC, hOldPen);
	SelectObject(hDC, hOldFont);
} /* end of DrawSect */

/*************************************************************************************/
int FAR hlvGetNewXrData(HWND hInkWnd, LPSTR lpXrData)
{
	/* This function is always called by recognizer
	   bGetNewXrData is set only if recognition was triggered by
	   Edit XR line dialog box */
	int               k;
#ifdef _PENWIN
	p_XRDATA        lpXR = (p_XRDATA)lpXrData;
#else
	int             size;
	RCBACCESS       lpfnRC;
	p_rec_info_type lpRCinfo;
	int             result;
	HINSTANCE       hDLL;
	LAB_XRDATA_TYPE xr;
#endif

	if (!bGetNewXrData)
	{
#ifdef _PENWIN
		lpXR->xr.xr = 0;
		return FALSE;
#endif
	}
	RETURN_IF_BAD_POINTER(lpNewXrLine);
	// copy data
	k = 0;
#ifdef _PENWIN
	while(lpNewXrLine[k].xr != 0)
	{
		lpXR[k].xr.xr       = lpNewXrLine[k].xr;
		lpXR[k].xr.a        = lpNewXrLine[k].a;
		lpXR[k].xr.p        = lpNewXrLine[k].p;
		lpXR[k].xr.h        = lpNewXrLine[k].h;
		lpXR[k].xr.begpoint = lpNewXrLine[k].begpoint;
		lpXR[k].xr.endpoint = lpNewXrLine[k].endpoint;
		lpXR[k].box_left  = 0;
		lpXR[k].box_up   = 0;
		lpXR[k].box_right = 0;
		lpXR[k].box_down = 0;
		k++;
	}
	lpXrData[k].xr.xr = 0;
	bGetNewXrData = FALSE;
#else
	hDLL = recGetDLLHandle();
	RETURN_IF_BAD_POINTER(hDLL);
	lpfnRC = (RCBACCESS) Q_GetProcAddress(hDLL, REC_DATA_ACCESS);
	RETURN_IF_BAD_POINTER(lpfnRC);
	lpRCinfo = recGetRCinfo();
	RETURN_IF_BAD_POINTER(lpRCinfo);


	// set new xr data for recognizer
	if (!bGetNewXrData)
	{
		// set first XR element to zero
		_fmemset(&xr, 0, sizeof(LAB_XRDATA_TYPE));
		result = (*lpfnRC)((void FAR *)lpRCinfo, OBJ_XRDATA, CMD_SET, 1, (ULONG) &xr);
		RETURN_IF_BAD_RESULT(result, 0);
	}
	else
	{
		bGetNewXrData = FALSE;
		size = 0;
		while (lpNewXrLine[size].xdxr.xnxr != 0)
		{
			size++;
		}
		result = (*lpfnRC)((void FAR *)lpRCinfo, OBJ_XRDATA, CMD_SET, (UINT) size, (ULONG) lpNewXrLine);
		DISPOSE(lpNewXrLine, "WGHLV hlvGetNewXrData");
		RETURN_IF_BAD_RESULT(result, 0);
	}
#endif
	return TRUE;
} /* end of hlvGetNewXrData */

/*************************************************************************************/
void FAR hlvKillXrData(void)
{
	DISPOSE(lpNewXrLine, "WGHLV hlvKillXrData");
} /* end of hlvKillXrData */

/*************************************************************************************/
BOOL FAR hlvGetSymbGraph(LPSTR buff, short FAR *sel_beg, short FAR *sel_end)
{
	HGLOBAL           hHlv;
	LP_HLV            lpHlv;
	LP_HLV_SECT       lpSect;
	int               k, len;
	p_GRAPH_TYPE      graph;
	BOOL              result = FALSE;

	if (!hLastDebugWnd)
	{
		return result;
	}
	hHlv = (HGLOBAL) GetWindowLong(GetDlgItem(hLastDebugWnd, DRAWINK), GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);
	lpSect = lpHlv->lpCurSect;
	graph = &lpSect->graph;
	if (prfShowGraph() && graph && graph->type == LAB_RWGT_SEPLET)
	{
		len = 0;
		for (k = 0; k < (int) graph->size; k++)
		{
			if (GRTYPE(graph->prwsb, k) == LAB_RWST_SPLIT)
			{
				buff[len] = GRSYMBOL(graph->prwsb, k + 1);
				buff[len + 1] = '\0';
				len++;
			}
			if (graph->pselb[k])
			{
				*sel_beg = len - 1;
			}
		}
		*sel_end = (*sel_beg < lstrlen(buff) - 2) ? (*sel_beg + 1) : (*sel_beg);
		result = TRUE;
	}
	else
	{
		result = FALSE;
	}
	DebugUnlockHandle(hHlv);
	return result;
} /* end of hlvGetSymbGraph */

/*************************************************************************************/
BOOL FAR hlvAllocCommData(HWND hWnd, p_TRACEDATA FAR *lpData)
{

	hlvKillAllocCommData();
	traceData.TDpt = (LPPOINT) DebugAllocPtr(GSHND, LAB_MAX_TRACE*sizeof(POINT),
	                 "WGHLV hlvAllocCommData");
	*lpData = &traceData;
	return (traceData.TDpt != NULL);
} /* end of hlvAllocCommData */

/*************************************************************************************/
void FAR hlvKillAllocCommData(void)
{

	if (traceData.TDpt != NULL)
	{
		DebugFreePtr(traceData.TDpt, "WGHLV hlvKillAllocCommData");
		traceData.TDnp = 0;
		traceData.TDpt = NULL;
	}
} /* end of hlvKillAllocCommData */

/*************************************************************************************/
BOOL FAR hlvRecognizeCommData(void)
{
#ifdef _PENWIN
	HWND              hWnd;
	HGLOBAL           hHlv;
	LP_HLV            lpHlv;
	LP_HLV_SECT       lpSect;

	if (hLastDebugWnd == NULL)
	{
		return FALSE;
	}
	hHlv = (HGLOBAL) GetWindowLong(GetDlgItem(hLastDebugWnd, DRAWINK), GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);
	lpSect = lpHlv->lpCurSect;
	hWnd = GetDlgItem(hLastDebugWnd, DRAWINK );
	// if trajectory is too small - free memory and return
	if (traceData.TDnp < MIN_COM_INPUT)
	{
		hlvKillAllocCommData();
		DebugUnlockHandle(hHlv);
		return FALSE;
	}
	FillTrace(lpSect, NULL, 0);
	gCurSect = -1;
	recRecognizeTrace(&lpSect->trace, NULL, FALSE, FALSE, 0);
	lpSect = lpHlv->lpCurSect->prev;
	RemoveSection(lpHlv, lpSect);
	DebugUnlockHandle(hHlv);
	return TRUE;
#else
	return TRUE;
#endif
} /* end of hlvRecognizeCommData */

/*************************************************************************************/
static void RemoveSection(LP_HLV lpHlv, LP_HLV_SECT lpSect)
{
	LP_HLV_SECT lpCurSect;

	lpCurSect = lpHlv->head;
	while (lpCurSect != lpSect && lpCurSect != NULL)
	{
		lpCurSect = lpCurSect->next;
	}
	if (lpCurSect == NULL)
	{
		return;
	}
	if (lpCurSect->prev == NULL)
	{
		// first section
		lpHlv->head = lpCurSect->next;
		lpCurSect->next->prev = NULL;
	}
	else
		if (lpCurSect->next == NULL)
		{
			// last section
			lpHlv->tail = lpCurSect->prev;
			lpCurSect->prev->next = NULL;
		}
		else
		{
			lpCurSect->prev->next = lpCurSect->next;
			lpCurSect->next->prev = lpCurSect->prev;
		}
	TrashSect(lpCurSect);
	DebugFreePtr(lpCurSect, "WGHLV RemoveSection");
} /* end of RemoveSection */

/*************************************************************************************/
void FAR hlvSortGraph(HWND hWnd)
{
	HGLOBAL           hHlv;
	LP_HLV            lpHlv;
	LP_HLV_SECT       lpSect;
	p_GRAPH_TYPE     graph;
	int               d_user;
	UINT              k, i, j;

	hHlv = (HGLOBAL) GetWindowLong(GetDlgItem(hLastDebugWnd, DRAWINK), GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);
	lpSect = lpHlv->lpCurSect;
	graph = &lpSect->graph;
	d_user = -1;
	for (k = 0; k < graph->size; k++)
	{
		if (GRTYPE(graph->prwsb, k) == LAB_RWST_SYM)
		{
			if (d_user < 0)
			{
				//first symbol in graph
				i = 0;
				j = 0;
				graph->pselb[k] = 1;
			}
			else
			{
				if (GRUSER(graph->prwsb, k) == d_user)
				{
					// same column
					if (GRTYPE(graph->prwsb, k - 1) != LAB_RWST_SYM)
						// not "glued" symbols, begin new row
					{
						i++;
					}
				}
				else
				{
					// begin new column and row
					i = 0;
					j++;
				}
			}
			d_user = GRUSER(graph->prwsb, k);
			graph->pnumb[k] = MAKELONG(i, j);
		}
		else
		{
			GRUSER(graph->prwsb, k) = (BYTE) -1;
		}
	}
	DebugUnlockHandle(hHlv);
} /* end of hlvSortGraph */

/*************************************************************************************/
static int SelectAnswer(LP_HLV lpHlv, WORD y)
{
	int     h, nFontSize, nMaxWords, answer, i;
	BOOL    ShowGraph;
	char    recword[LAB_RW_SIZE + 1];

	h = lpHlv->rcAnswer.bottom - lpHlv->rcAnswer.top;
	nMaxWords = lpHlv->lpCurSect->n_answers > lpHlv->lpCurSect->n_rec_words ?
	            lpHlv->lpCurSect->n_answers : lpHlv->lpCurSect->n_rec_words;
	ShowGraph = (prfShowGraph() &&
	             lpHlv->lpCurSect->graph.type == LAB_RWGT_SEPLET);
	if (ShowGraph)
	{
		nMaxWords = LAB_XR_SIZE + 1;
	}
	nFontSize = (h - 2) / (nMaxWords + 1);
	h = y - lpHlv->rcAnswer.top;
	answer = h / nFontSize;
	i = -1;
	do
	{
		i++;
#ifdef _PENWIN
		lstrcpy(recword, (LPSTR)lpHlv->lpCurSect->rec_words[i].word);
#else
		lstrcpy(recword, (LPSTR) lpHlv->lpCurSect->rec_words[i].rwword);
#endif
	}
	while (lstrlen(recword) != 0);
	i -= 1;
	answer = answer > i ? -1 : answer;
	return answer;
} /* end of SelectAnswer */

/*************************************************************************************/
BOOL FAR hlvTraceWindowHit(HWND hWnd, LPARAM lParam)
{
	HGLOBAL           hHlv;
	LP_HLV            lpHlv;
	LP_HLV_SECT       lpSect;
	BOOL              result;
	int               x, y;
	DLGPROC           dlgprc;
	int               answer;

	hHlv = (HGLOBAL) GetWindowLong(GetDlgItem(hLastDebugWnd, DRAWINK), GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);
	lpSect = lpHlv->lpCurSect;
	result = FALSE;

#ifndef _PENWIN
	if (/*srlAwaitInput() || */lpHlv->lpCurSect == NULL)
	{
		DebugUnlockHandle(hHlv);
		return FALSE;
	}
#endif


	x = (int) LOWORD(lParam);
	y = (int) HIWORD(lParam);
	if (x > lpHlv->rcPAnswer.left && y > lpHlv->rcPAnswer.top && DebugLevel < 0)
	{
		dlgprc = (DLGPROC) MakeProcInstance((FARPROC) hlvAnswersDlgBox, hInst);
		if (prfCorrectOneWord())
		{
			answer = SelectAnswer(lpHlv, HIWORD(lParam));
			if (answer >= 0)
			{
				lpSect->selcor = answer;
				if (hCorrectDlgBox == NULL)
					hCorrectDlgBox = CreateDialogParam(hInst, "CORRECTWORD", hLastDebugWnd,
					                                   dlgprc, (LPARAM) lpSect);
				else
				{
					SetWindowsData(hCorrectDlgBox, ID_LETTERS, lpSect, TRUE, FALSE);
					InvalidateRect(hCorrectDlgBox, NULL, TRUE);
					UpdateWindow(hCorrectDlgBox);
				}
			}
		}
		else
		{
			lpSect->selcor = -1;
			hAnswerDlgBox = CreateDialogParam(hInst, "CORRECT", hLastDebugWnd,
			                                  dlgprc, (LPARAM) lpSect);
		}
	}
	else
		if (x < lpHlv->rcGraph.left)
		{
			result = TRUE;
		}
		else
			if (y < lpHlv->rcGraph.top)
			{
				result = TRUE;
			}
	DebugUnlockHandle(hHlv);
	return result;
} /* end of hlvTraceWindowHit */

/*************************************************************************************/
static void AddWordToGraph(LPSTR prws, LPSTR Word, UINT *j)
{
#ifdef _PENWIN
	p_RWS_type FAR *rws = (p_RWS_type FAR *)prws;
#else
	p_LAB_RWGNODE   rws = (p_LAB_RWGNODE) prws;
#endif

	GRTYPE(rws, *j) = (*j == 0) ? LAB_RWST_SPLIT : LAB_RWST_NEXT;
	GRUSER(rws, *j) = 1;
	(*j)++;
	AddOneWordToGraph(prws, Word, j);
	GRTYPE(rws, *j) = LAB_RWST_JOIN;
	GRUSER(rws, *j) = 1;
} /* end of AddWordToGraph */

/*************************************************************************************/
static void AddOneWordToGraph(LPSTR prws, LPSTR Word, UINT *j)
{
#ifdef _PENWIN
	p_RWS_type FAR *rws = (p_RWS_type FAR *)prws;
#else
	p_LAB_RWGNODE   rws = (p_LAB_RWGNODE) prws;
#endif
	UINT    i;

	for (i = 0; Word[i] != 0; i++)
	{
		GRTYPE(rws, *j) = LAB_RWST_SYM;
		GRSYMBOL(rws, *j) = Word[i];
		GRUSER(rws, *j) = 1;
		(*j)++;
	}
} /* end of AddWordToGraph */

/*************************************************************************************/
static int FreeGraph(p_GRAPH_TYPE pGraph)
{

	DISPOSE(pGraph->prwsb, "WGHLV FreeGraph");
	DISPOSE(pGraph->prwse, "WGHLV FreeGraph");
	DISPOSE(pGraph->pppdb, "WGHLV FreeGraph");
	DISPOSE(pGraph->pppde, "WGHLV FreeGraph");
	DISPOSE(pGraph->pselb, "WGHLV FreeGraph");
	DISPOSE(pGraph->psele, "WGHLV FreeGraph");
	DISPOSE(pGraph->pnumb, "WGHLV FreeGraph");
	DISPOSE(pGraph->pnume, "WGHLV FreeGraph");
	DISPOSE(pGraph->prwsb, "WGHLV FreeGraph");
	pGraph->size = 0;
	return TRUE;
}  /* end of FreeGraph() */

/*************************************************************************************/
static int AllocNewGraph(p_GRAPH_TYPE pGraph, int size)
{
	// alloc space  8 arrays : prwsb, pppdb - rws & ppd before postproccrsing
	//                         prwse, pppde - rws & ppd after postproccrsing
	//                         pselb, pnumb - auxilary arrays for PENLAB before postproccrsing
	//                         psele, pnume - auxilary arrays for PENLAB after postproccrsing
	pGraph->prwsb = (p_LAB_RWGNODE) DebugAllocPtr(GSHND, sizeof(LAB_RWGNODE_TYPE)*size,
	                "WGHLV AllocNewGraph");
	RETURN_IF_BAD_POINTER(pGraph->prwsb);
	pGraph->prwse = (p_LAB_RWGNODE) DebugAllocPtr(GSHND, sizeof(LAB_RWGNODE_TYPE)*size,
	                "WGHLV AllocNewGraph");
	RETURN_IF_BAD_POINTER(pGraph->prwse);
	pGraph->pppdb = (p_LAB_PPNODE) DebugAllocPtr(GSHND, sizeof(LAB_RWG_PPD_ARRAY)*size,
	                "WGHLV AllocNewGraph");
	RETURN_IF_BAD_POINTER(pGraph->pppdb);
	pGraph->pppde = (p_LAB_PPNODE) DebugAllocPtr(GSHND, sizeof(LAB_RWG_PPD_ARRAY)*size,
	                "WGHLV AllocNewGraph");
	RETURN_IF_BAD_POINTER(pGraph->pppde);
	pGraph->pselb = (p_UCHAR) DebugAllocPtr(GSHND, sizeof(char)*size,
	                                        "WGHLV AllocNewGraph");
	RETURN_IF_BAD_POINTER(pGraph->pselb);
	pGraph->psele = (p_UCHAR) DebugAllocPtr(GSHND, sizeof(char)*size,
	                                        "WGHLV AllocNewGraph");
	RETURN_IF_BAD_POINTER(pGraph->psele);
	pGraph->pnumb = (p_ULONG) DebugAllocPtr(GSHND, sizeof(LONG)*size,
	                                        "WGHLV AllocNewGraph");
	RETURN_IF_BAD_POINTER(pGraph->pnumb);
	pGraph->pnume = (p_ULONG) DebugAllocPtr(GSHND, sizeof(LONG)*size,
	                                        "WGHLV AllocNewGraph");
	RETURN_IF_BAD_POINTER(pGraph->pnumb);
	return TRUE;
} /* end of AllocNewGraph() */

/*************************************************************************************/
static int CopyGraph(p_GRAPH_TYPE pDestGraph, p_GRAPH_TYPE pSrcGraph)
{
	int   size;

	size = pDestGraph->size = pSrcGraph->size;
	if (AllocNewGraph(pDestGraph, size))
	{
		CopyGraphData((LPSTR) pDestGraph->prwsb, (LPSTR) pSrcGraph->prwsb,
		              0, sizeof(LAB_RWGNODE_TYPE)*size);
		CopyGraphData((LPSTR) pDestGraph->prwse, (LPSTR) pSrcGraph->prwse,
		              0, sizeof(LAB_RWGNODE_TYPE)*size);
		CopyGraphData((LPSTR) pDestGraph->pppdb, (LPSTR) pSrcGraph->pppdb,
		              0, sizeof(LAB_RWG_PPD_ARRAY)*size);
		CopyGraphData((LPSTR) pDestGraph->pppde, (LPSTR) pSrcGraph->pppde,
		              0, sizeof(LAB_RWG_PPD_ARRAY)*size);
		CopyGraphData((LPSTR) pDestGraph->pselb, (LPSTR) pSrcGraph->pselb,
		              0, sizeof(char)*size);
		CopyGraphData((LPSTR) pDestGraph->psele, (LPSTR) pSrcGraph->psele,
		              0, sizeof(char)*size);
		CopyGraphData((LPSTR) pDestGraph->pnumb, (LPSTR) pSrcGraph->pnumb,
		              0, sizeof(LONG)*size);
		CopyGraphData((LPSTR) pDestGraph->pnume, (LPSTR) pSrcGraph->pnume,
		              0, sizeof(LONG)*size);
		return TRUE;
	}
	return FALSE;
} /* end of CopyGraph() */

/*************************************************************************************/
static int CopyGraphData(LPSTR Dest, LPSTR Src, int BeginIndex, int size)
{
	int   i, j;

	i = BeginIndex;
	j = 0;
	while (j < size)
	{
		Dest[i++] = Src[j++];
	}
	return i;
} /* end of CopyGraphData() */

/*************************************************************************************/
static int MergeGraphs(p_GRAPH_TYPE pDest, p_GRAPH_TYPE p1, p_GRAPH_TYPE p2, LPSTR Word)
{
	int     i;
	LPSTR   temp;

	if (Word != NULL)
	{
		p1->size = lstrlen(Word) + 2; // { + }
	}
	// skip first { in second graph
	p2->size = p2->size - 1;
	if (AllocNewGraph(pDest, p1->size + p2->size + 1))
	{
		// skeep WordNum word
		pDest->size = p1->size + p2->size;
		i = 0;
		i = CopyGraphData((LPSTR) pDest->prwsb, (LPSTR) p1->prwsb, i, sizeof(LAB_RWGNODE_TYPE)*p1->size);
		temp = ((LPSTR) (p2->prwsb) + sizeof(LAB_RWGNODE_TYPE));
		i = CopyGraphData((LPSTR) pDest->prwsb, temp, i, sizeof(LAB_RWGNODE_TYPE)*p2->size);
		i = 0;
		i = CopyGraphData((LPSTR) pDest->prwse, (LPSTR) p1->prwse, i, sizeof(LAB_RWGNODE_TYPE)*p1->size);
		temp = ((LPSTR) (p2->prwse) + sizeof(LAB_RWGNODE_TYPE));
		i = CopyGraphData((LPSTR) pDest->prwse, temp, i, sizeof(LAB_RWGNODE_TYPE)*p2->size);
		i = 0;
		i = CopyGraphData((LPSTR) pDest->pppdb, (LPSTR) p1->pppdb, i, sizeof(LAB_RWG_PPD_ARRAY)*p1->size);
		temp = ((LPSTR) (p2->pppdb) + sizeof(LAB_RWG_PPD_ARRAY));
		i = CopyGraphData((LPSTR) pDest->pppdb, temp, i, sizeof(LAB_RWG_PPD_ARRAY)*p2->size);
		i = 0;
		i = CopyGraphData((LPSTR) pDest->pppde, (LPSTR) p1->pppde, i, sizeof(LAB_RWG_PPD_ARRAY)*p1->size);
		temp = ((LPSTR) (p2->pppde) + sizeof(LAB_RWG_PPD_ARRAY));
		i = CopyGraphData((LPSTR) pDest->pppde, temp, i, sizeof(LAB_RWG_PPD_ARRAY)*p2->size);
		i = 0;
		i = CopyGraphData((LPSTR) pDest->pselb, (LPSTR) p1->pselb, i, sizeof(char)*p1->size);
		temp = ((LPSTR) (p2->pselb) + sizeof(char));
		i = CopyGraphData((LPSTR) pDest->pselb, temp, i, sizeof(char)*p2->size);
		i = 0;
		i = CopyGraphData((LPSTR) pDest->psele, (LPSTR) p1->psele, i, sizeof(char)*p1->size);
		temp = ((LPSTR) (p2->psele) + sizeof(char));
		i = CopyGraphData((LPSTR) pDest->psele, temp, i, sizeof(char)*p2->size);
		i = 0;
		i = CopyGraphData((LPSTR) pDest->pnumb, (LPSTR) p1->pnumb, i, sizeof(LONG)*p1->size);
		temp = ((LPSTR) (p2->pnumb) + sizeof(LONG));
		i = CopyGraphData((LPSTR) pDest->pnumb, temp, i, sizeof(LONG)*p2->size);
		i = 0;
		i = CopyGraphData((LPSTR) pDest->pnume, (LPSTR) p1->pnume, i, sizeof(LONG)*p1->size);
		temp = ((LPSTR) (p2->pnume) + sizeof(LONG));
		i = CopyGraphData((LPSTR) pDest->pnume, temp, i, sizeof(LONG)*p2->size);
		return TRUE;
	}
	return FALSE;
} /* end of MergeGraphs() */

/*************************************************************************************/
BOOL FAR hlvBuildNewGraph(HWND hWnd, LPSTR Word, BOOL SpecialMode)
{
#ifndef _PENWIN
	HGLOBAL           hHlv;
	LP_HLV            lpHlv;
	LP_HLV_SECT       lpSect;
	p_GRAPH_TYPE      graph;
	p_rec_info_type   pRCinfo;
	int               result = FALSE;
	RCBACCESS         lpfnRC;
	HINSTANCE         hDLL;
	char              recword[LAB_RW_SIZE];
	int                 i, j;
	UINT                size;
	p_LAB_RWGNODE     prwsb;

	LAYOUTWORD_TYPE     WordLayOut;
	GRAPH_TYPE          SavedGraph;
	GRAPH_TYPE          NewGraph;
	pRCinfo = recGetRCinfo();
	RETURN_IF_BAD_POINTER(pRCinfo);
	hDLL = recGetDLLHandle();
	RETURN_IF_BAD_POINTER(hDLL);
	lpfnRC = (RCBACCESS) Q_GetProcAddress(hDLL, REC_DATA_ACCESS);
	pRCinfo = recGetRCinfo();
	RETURN_IF_BAD_POINTER(lpfnRC);

	hHlv = (HGLOBAL) GetWindowLong(GetDlgItem(hLastDebugWnd, DRAWINK), GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);
	if (lpHlv->lpCurSect == NULL)
	{
		DebugUnlockHandle(hHlv);
		return result;
	}

	graph = &lpHlv->lpCurSect->graph;
	lpSect = lpHlv->lpCurSect;

	RETURN_IF_BAD_POINTER(lpSect->answers);

	if (lpSect->rec_words[0].rwword[0] == 0)
	{
		DebugUnlockHandle(hHlv);
		return result;
	}

	lstrcpy((LPSTR) lpSect->rec_words[0].rwword, Word);


	// get memory for the new graph with inserted new word
	size = (UINT) lstrlen(Word) + 2;
	i = 1;
	while (i < lpSect->n_rec_words && lpSect->rec_words[i].rwword[0])
	{
		lstrcpy(recword, (LPSTR) lpSect->rec_words[i].rwword);
		size += lstrlen(recword) + 2;
		i++;
	}

	if (SpecialMode)
	{
		result = CopyGraph(&SavedGraph, graph);
		if (!result)
		{
			DebugUnlockHandle(hHlv);
			return result;
		}
	}
	prwsb = (p_LAB_RWGNODE) DebugAllocPtr(GSHND, graph->size*sizeof(LAB_RWGNODE_TYPE),
	                                      "WGHLV hlvBuildNewGraph");
	if (prwsb == NULL)
	{
		DebugUnlockHandle(lpHlv);
		return FALSE;
	}

	// save old graph
	_fmemcpy(prwsb, lpSect->graph.prwsb, graph->size*sizeof(LAB_RWGNODE_TYPE));

	DISPOSE(lpSect->graph.prwsb, "WGHLV hlvBuildNewGraph()");
	lpSect->graph.prwsb = (p_LAB_RWGNODE) DebugAllocPtr(GSHND,
	                      size*sizeof(LAB_RWGNODE_TYPE),
	                      "WGHLV hlvBuildNewGraph");
	if (lpSect->graph.prwsb == NULL)
	{
		lpSect->graph.prwsb = prwsb;
		DebugUnlockHandle(lpHlv);
		return FALSE;
	}

	// build new graph

	// i == number of none zero rec_words in lpSect->rec_words
	// if i == 1  - that is new graph will contain only one word,
	// we  must not put { and } symbols in graph
	j = 0;

	if (SpecialMode)
	{
		AddWordToGraph((LPSTR) lpSect->graph.prwsb, Word, (UINT*) &j);
		graph->size = j + 1;
	}
	else
		if (i == 1)
		{
			AddOneWordToGraph((LPSTR) lpSect->graph.prwsb, Word, (UINT*) &j);
			graph->size = lstrlen(Word);
		}
		else
		{
			AddWordToGraph((LPSTR) lpSect->graph.prwsb, Word, (UINT*) &j);
			i = 1;
			while (TRUE)
			{
				lstrcpy(recword, (LPSTR) lpSect->rec_words[i].rwword);
				if (lstrlen(recword) == 0)
				{
					break;
				}
				AddWordToGraph((LPSTR) lpSect->graph.prwsb, recword, (UINT*) &j);
				i++;
			}
			graph->size = j + 1;
		}
	graph->type = LAB_RWGT_WORD;
	result = (*lpfnRC)((void FAR *)pRCinfo,
	                   OBJ_RWGRAPH, CMD_SET, (UINT) lpSect->graph.size, (ULONG) &lpSect->graph);
	if (result <= 0)
	{
		// restore old graph
		DISPOSE(lpSect->graph.prwsb, "WGHLV hlvBuildNewGraph()");
		lpSect->graph.prwsb = prwsb;
		DebugUnlockHandle(hHlv);
		return FALSE;
	}
	WordLayOut.pRCInfo = pRCinfo;
	WordLayOut.SpMode = SpecialMode;
	lstrcpy(WordLayOut.Word, Word);
	WordLayOut.pxr = lpSect->xrs;

	{
		//CHE: for exact layout:
		_SHORT  caps_mode_saved = ((rc_type _PTR)(pRCinfo->prc))->caps_mode;
		((rc_type _PTR)(pRCinfo->prc))->caps_mode = XCM_AL_DEFSIZE;
		msgWordLayOut((LPSTR) &WordLayOut);
		((rc_type _PTR)(pRCinfo->prc))->caps_mode = caps_mode_saved;
	}
	if (SpecialMode)
	{
		// merge new and old graph
		MergeGraphs(&NewGraph, &lpSect->graph, &SavedGraph, Word);
		// kill lpSect graph
		FreeGraph(graph);
		// copy NewGraph into lpSect graph
		graph->size = NewGraph.size;
		graph->prwsb = NewGraph.prwsb;
		graph->prwse = NewGraph.prwse;
		graph->pppdb = NewGraph.pppdb;
		graph->pppde = NewGraph.pppde;
		graph->pselb = NewGraph.pselb;
		graph->psele = NewGraph.psele;
		graph->pnumb = NewGraph.pnumb;
		graph->pnume = NewGraph.pnume;
		FreeGraph(&SavedGraph);
		// merge answers
		i = 0;
		while (lpSect->answers[i].anword[0] != 0 && i < lpSect->n_answers)
		{
			i++;
		}
		i--;
		for (; i >= 0; i--)
		{
			lpSect->answers[i + 1] = lpSect->answers[i];
		}
	}
	lstrcpy(lpSect->answers[0].anword, Word);

	DISPOSE(prwsb, "WGHLV hlvBuildNewGraph");
	lpSect->selword = 0;
	DebugUnlockHandle(hHlv);
	InvalidateRect(hWnd, NULL, TRUE);
#endif
	return TRUE;
} /* end of hlvBuildNewGraph */

/*************************************************************************************/
void FAR hlvShowGraphDlgBox(HWND hWnd)
{
	DLGPROC           dlgprc;
	HGLOBAL           hHlv;
	LP_HLV            lpHlv;
	LP_HLV_SECT       lpSect;
	p_GRAPH_TYPE     graph;

	if (hLastDebugWnd == NULL)
	{
		return;
	}
	hHlv = (HGLOBAL) GetWindowLong(GetDlgItem(hLastDebugWnd, DRAWINK), GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);
	lpSect = lpHlv->lpCurSect;
	graph = &lpHlv->lpCurSect->graph;
	dlgprc = (DLGPROC) MakeProcInstance((FARPROC) hlvDlgBox, hInst);
	DialogBoxParam(hInst, "SHOWGRAPH", hWnd, dlgprc, (LPARAM) graph);
	FreeProcInstance((FARPROC) dlgprc);
	DebugLockHandle(hHlv);
} /* end of hlvShowGraphDlgBox */

/*************************************************************************************/
static BOOL GetGraphWord(p_GRAPH_TYPE graph, short *i, LPSTR buff)
{
	char            code[4];

	buff[0] = 0;
	if (graph->type == LAB_RWGT_SEPLET)
	{
		if (*i >= graph->size)
		{
			return FALSE;
		}
		while (*i < graph->size)
		{
			if (GRTYPE(graph->prwsb, *i) == LAB_RWST_SYM)
			{
				code[0] = GRSYMBOL(graph->prwsb, *i);
				code[1] = 0;
				lstrcat(buff, code);
			}
			else
				if (GRTYPE(graph->prwsb, *i) == LAB_RWST_SPLIT)
				{
					lstrcat(buff, " {");
				}
				else
					if (GRTYPE(graph->prwsb, *i) == LAB_RWST_NEXT)
					{
						lstrcat(buff, "|");
					}
					else
						if (GRTYPE(graph->prwsb, *i) == LAB_RWST_JOIN)
						{
							lstrcat(buff, "} ");
						}
			(*i)++;
		}
		return TRUE;
	}
	while (*i < graph->size)
	{
		if (GRTYPE(graph->prwsb, *i) == LAB_RWST_SYM)
		{
			code[1] = 0;
			code[0] = GRSYMBOL(graph->prwsb, *i);
			lstrcat(buff, code);
		}
		else
			if (GRTYPE(graph->prwsb, *i) == LAB_RWST_SPLIT)
			{
				lstrcat(buff, " {");
			}
			else
				if (GRTYPE(graph->prwsb, *i) == LAB_RWST_NEXT)
				{
					lstrcat(buff, "|");
					(*i)++;
					return TRUE;
				}
				else
					if (GRTYPE(graph->prwsb, *i) == LAB_RWST_JOIN)
					{
						lstrcat(buff, "}");
						(*i)++;
						return TRUE;
					}
		(*i)++;
	}
	return FALSE;
} /* end of GetGraphWord */

/*************************************************************************************/
LRESULT CALLBACK hlvDlgBox(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

	char              buff[_MAX_PATH];
	HWND              hWnd;
	p_GRAPH_TYPE     graph;
	short             i;

	switch (message)
	{
		case WM_INITDIALOG:
			graph = (p_GRAPH_TYPE) lParam;
			hWnd = GetDlgItem(hDlg, ID_TEXT);
			SendMessage(hWnd, WM_SETFONT, (WPARAM) GetStockObject(SYSTEM_FONT), (LPARAM) TRUE);
			SendMessage(hWnd, LB_SETHORIZONTALEXTENT, 800, 0L);
			i = 0;
			while (GetGraphWord(graph, &i, buff))
			{
				SendMessage(hWnd, LB_ADDSTRING, 0, (LPARAM) (LPSTR) buff);
			}
			wsprintf(buff, "Graph size = %d", graph->size);
			SendMessage(hWnd, LB_ADDSTRING, 0, (LPARAM) (LPSTR) buff);
			return TRUE;

		case WM_COMMAND:
			switch (GET_WM_COMMAND_ID(wParam, lParam))
			{
				case IDOK:
					EndDialog(hDlg, TRUE);
					return TRUE;
			}
			break;
	}
	return FALSE;
} /* end of hlvShowGraphDlgBox */

/*************************************************************************************/
void FAR hlvGetMouseRectangle(HWND hWnd, LPRECT rc)
{
	HGLOBAL         hHlv;
	LP_HLV          lpHlv;

	if (hWnd == NULL)
	{
		return;
	}
	hHlv = (HGLOBAL) GetWindowLong(hWnd, GWW_HLV_DATA);
	if (hHlv == NULL)
	{
		GetClientRect(hWnd, rc);
		return;
	}
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);
	if (lpHlv->lpCurSect == NULL)
	{
		GetClientRect(hWnd, rc);
		DebugLockHandle(hHlv);
		return;
	}
	CopyRect(rc, &(lpHlv->rcInk));
	if (IsRectEmpty(rc))
	{
		GetClientRect(hWnd, rc);
	}
	else
	{
		rc->bottom = lpHlv->rcXrs.bottom;
	}
} /* end of hlvGetMouseRectangle */

/******************************************************************************************/
static void SaveCorrection(HWND hWnd, int index, LPSTR Answer)
{
	HGLOBAL         hHlv;
	LP_HLV          lpHlv;

	if (hWnd == NULL)
	{
		return;
	}
	hHlv = (HGLOBAL) GetWindowLong(hWnd, GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);
	if (lpHlv->lpCurSect != NULL)
	{
#ifdef _PENWIN
		lstrcpy(lpHlv->lpCurSect->rec_words[index].word, Answer);
#else
		lstrcpy(lpHlv->lpCurSect->rec_words[index].rwword, Answer);
#endif
		InvalidateRect(hWnd, &lpHlv->rcPAnswer, TRUE);
	}
	DebugUnlockHandle(hHlv);
} /* end of SaveCorrection */

BOOL bFirstStroke;

/******************************************************************************************/
LRESULT CALLBACK hlvAnswersProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT       ps;
	HDC               hDC;
	HGLOBAL           hData;
	p_CORANSW_TYPE    lpAnswers;
	RECT              rc;
	static  RECT      BoundingRect;
	static  BOOL      bChangeFont = FALSE;
	static  DWORD     line_col;
	static BOOL       bBeginWriting = FALSE;

	hData = (HGLOBAL) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
	if (hData == NULL)
	{
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	lpAnswers = (p_CORANSW_TYPE) DebugLockHandle(hData);
	if (lpAnswers == NULL)
	{
		DebugUnlockHandle(hData);
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	switch (message)
	{
		case WM_LBUTTONDBLCLK:
			PostMessage(GetParent(hWnd), WM_COMMAND, IDOK, 0L);
			break;

		case WM_MOUSEMOVE:
			if (bBeginWriting && (wParam & MK_LBUTTON))
			{
				HGLOBAL hHlv = (HGLOBAL) GetWindowLong(GetDlgItem(hLastDebugWnd, DRAWINK), GWW_HLV_DATA);
				LP_HLV  lpHlv = (LP_HLV) DebugLockHandle(hHlv);
				if (lpHlv == NULL)
				{
					break;
				}
				if (lpHlv->lpCurSect == NULL)
				{
					DebugUnlockHandle(hHlv);
					break;
				}
				{
					LP_HLV_SECT lpSect = lpHlv->lpCurSect;
					int NumPoints = lpSect->np_lo_trace;
					int xCur = LOWORD(lParam);
					int yCur = HIWORD(lParam);
					int xPrv = lpSect->lo_trace[NumPoints - 1].x;
					int yPrv = lpSect->lo_trace[NumPoints - 1].y;
					lpSect->lo_trace[NumPoints].x = xCur;
					lpSect->lo_trace[NumPoints++].y = yCur;
					lpSect->np_lo_trace = NumPoints;
					if (xCur != xPrv || yCur != yPrv)
					{
						HDC hDC = GetDC(hWnd);
						MoveToEx(hDC, xPrv, yPrv, NULL);
						LineTo(hDC, xCur, yCur);
						ReleaseDC(hWnd, hDC);
					}
				}
				DebugUnlockHandle(hHlv);
			}
			break;

		case WM_LBUTTONDOWN:
		{
			HGLOBAL hHlv = (HGLOBAL) GetWindowLong(GetDlgItem(hLastDebugWnd, DRAWINK), GWW_HLV_DATA);
			LP_HLV  lpHlv = (LP_HLV) DebugLockHandle(hHlv);
			if (lpHlv == NULL)
			{
				break;
			}
			if (lpHlv->lpCurSect == NULL)
			{
				DebugUnlockHandle(hHlv);
				break;
			}
			{
				LP_HLV_SECT lpSect = lpHlv->lpCurSect;
				int NumPoints = lpSect->np_lo_trace;
				bBeginWriting = TRUE;
				if (bFirstStroke)
				{
					lpSect->lo_trace[NumPoints].x = 0;
					lpSect->lo_trace[NumPoints++].y = -1;
					bFirstStroke = FALSE;
				}
				lpSect->lo_trace[NumPoints].x = LOWORD(lParam);
				lpSect->lo_trace[NumPoints++].y = HIWORD(lParam);
				lpSect->np_lo_trace = NumPoints;
				DebugUnlockHandle(hHlv);
			}
		}
		break;

		case WM_LBUTTONUP:
		{
			HGLOBAL hHlv = (HGLOBAL) GetWindowLong(GetDlgItem(hLastDebugWnd, DRAWINK), GWW_HLV_DATA);
			LP_HLV  lpHlv = (LP_HLV) DebugLockHandle(hHlv);
			if (lpHlv == NULL)
			{
				break;
			}
			if (lpHlv->lpCurSect == NULL)
			{
				DebugUnlockHandle(hHlv);
				break;
			}
			{
				LP_HLV_SECT lpSect = lpHlv->lpCurSect;
				int NumPoints = lpSect->np_lo_trace;
				bBeginWriting = FALSE;
				lpSect->lo_trace[NumPoints].x = 0;
				lpSect->lo_trace[NumPoints++].y = -1;
				lpSect->np_lo_trace = NumPoints;
				DebugUnlockHandle(hHlv);
			}
		}
		break;

		case  WM_PAINT:
			if (lpAnswers->n_rec_words > 0)
			{
				HGLOBAL hHlv = (HGLOBAL) GetWindowLong(GetDlgItem(hLastDebugWnd, DRAWINK), GWW_HLV_DATA);
				LP_HLV  lpHlv = (LP_HLV) DebugLockHandle(hHlv);
				if (lpHlv == NULL)
				{
					break;
				}
				if (lpHlv->lpCurSect == NULL)
				{
					DebugUnlockHandle(hHlv);
					break;
				}
				{
					LP_HLV_SECT lpSect = lpHlv->lpCurSect;
					GetClientRect(hWnd, &rc);
					hDC = BeginPaint(hWnd, &ps);
					DrawAnswers(hDC, &rc, lpAnswers, bChangeFont);
					Polyline(hDC, lpSect->lo_trace, lpSect->np_lo_trace);
					bChangeFont = FALSE;
					EndPaint(hWnd, &ps);
					DebugUnlockHandle(hHlv);
				}
			}
			break;

		case WM_CLOSE:
			if (lpAnswers->hFont)
			{
				DeleteObject(lpAnswers->hFont);
			}
			bChangeFont = FALSE;
			DebugUnlockHandle(hData);
			DebugFree(hData, "WGHLV hlvAnswersProc");
			return DefWindowProc(hWnd, message, wParam, lParam);

		default:
			break;

	}
	DebugUnlockHandle(hData);
	return DefWindowProc(hWnd, message, wParam, lParam);
} /* end of hlvAnswersProc */

/******************************************************************************************/
static void DrawAnswers(HDC hDC, LPRECT lprc, p_CORANSW_TYPE lpAnswers, BOOL bChangeFont)
{
	RECT          rc;
	int           i, lines, index;
	LOGFONT       lf;
	HFONT         hOldFont;
	p_LAB_RECWORD p_rec;
	WORD          limit;

	CopyRect(&rc, lprc);
	lpAnswers->max_length = 0;
	//  p_rec = (p_LAB_RECWORD)&(lpAnswers->rec_words);
	p_rec = (p_LAB_RECWORD) &(lpAnswers->rec_words[0]);
	i = 0;
	limit = lpAnswers->word_index < 0 ? MAX_DLG_ANSWERS : MIN_DLG_ANSWERS;
	if (lpAnswers->word_index <= 0)
	{
		index = 0;
	}
	else
	{
		index = lpAnswers->word_index;
	}
	p_rec = (p_LAB_RECWORD) &(lpAnswers->rec_words[index]);
	/**
	  offset = ((lpAnswers->word_index < 0) ? 0 : lpAnswers->word_index)*sizeof(LAB_RECWORD_TYPE);
	  p_rec = (p_LAB_RECWORD)((LPSTR)p_rec + offset);
	  //  p_rec += ((lpAnswers->word_index < 0) ? 0 : lpAnswers->word_index);
	  **/
	while (i < limit)
	{
		lines = lstrlen(p_rec[i].rwword);
		if (lines == 0)
		{
			break;
		}
		lpAnswers->max_length = max(lpAnswers->max_length, lines);
		i++;
	}
	lpAnswers->n_rec_words = i;
	if (lpAnswers->n_rec_words == 0)
	{
		return;
	}
	//  rc.right  -= DLG_OFFSET; rc.left += DLG_OFFSET;
	//  rc.bottom -= DLG_OFFSET; rc.top  += DLG_OFFSET;
	lpAnswers->guide.xOrigin = 0;
	lpAnswers->guide.yOrigin = 0;
	lpAnswers->guide.cxBox = (rc.right - rc.left) / (lpAnswers->max_length * 2 + 1);
	lpAnswers->guide.cyBox = (rc.bottom - rc.top) / lpAnswers->n_rec_words;
	lpAnswers->guide.cyBase = lpAnswers->guide.cyBox * 2 / 3;  // *4/5; CHE
	lpAnswers->guide.cxBase = max(0, (lpAnswers->guide.cxBox - lpAnswers->guide.cyBase * 3 / 4) / 2);
	lpAnswers->guide.cHorzBox = 1;
	lpAnswers->guide.cVertBox = 1;
	lpAnswers->guide.cyMid = lpAnswers->guide.cyBox / 4; //lpAnswers->guide.cyBase/2; //*2/3; CHE
	SelectObject(hDC, GetStockObject(ANSI_FIXED_FONT));
	if (lpAnswers->hFont == NULL || bChangeFont)
	{
		if (lpAnswers->hFont)
		{
			DeleteObject(lpAnswers->hFont);
		}
		_fmemset(&lf, 0, sizeof(LOGFONT));
		lf.lfHeight = -lpAnswers->guide.cyBase * 6 / 7;   //CHE
		lf.lfPitchAndFamily = FIXED_PITCH;
		lstrcpy(lf.lfFaceName, "Courier New (True Type)");
		lpAnswers->hFont = CreateFontIndirect(&lf);
	}
	if (lpAnswers->hFont)
	{
		hOldFont = (HFONT) SelectObject(hDC, lpAnswers->hFont);
	}
	SetBkMode(hDC, TRANSPARENT);
	i = 0;
	while (i < lpAnswers->n_rec_words)
	{
		DrawGuidesAndAnswers(hDC, lpAnswers, &rc, i);
		i++;
	}
	if (lpAnswers->hFont)
	{
		SelectObject(hDC, hOldFont);
	}
} /* end of DrawAnswers */

/******************************************************************************************/
static void SetTrajectoryGuide(HWND hWnd, p_CORANSW_TYPE lpAnswers,
                               LPGUIDE guide, DWORD *line_col, RECT BoundingRect)
{
	RECT      rc;
	int       xc, yc;
	WORD      line, col;
	POINT     pt;

	_fmemcpy(guide, &lpAnswers->guide, sizeof(GUIDE));
	GetClientRect(hWnd, &rc);
	xc = (BoundingRect.right + BoundingRect.left) / 2;
	yc = (BoundingRect.bottom + BoundingRect.top) / 2;
	line = (yc - DLG_OFFSET) / lpAnswers->guide.cyBox;
	col = (xc - DLG_OFFSET) / lpAnswers->guide.cxBox;
	*line_col = line;
	*line_col <<= 16;
	*line_col += col;
	pt.x = rc.left + col * lpAnswers->guide.cxBox + DLG_OFFSET;
	pt.y = rc.top + line * lpAnswers->guide.cyBox + DLG_OFFSET;
	ClientToScreen(hWnd, &pt);
	guide->xOrigin = (short) pt.x;
	guide->yOrigin = (short) pt.y;
} /* end of SetTrajectoryGuide */

/******************************************************************************************/
static void DrawResult(HWND hWnd, p_CORANSW_TYPE lpAnswers, char Answer,
                       DWORD line_col, BOOL *bChangeFont)
{
	HDC           hDC;
	int           word_num, letter_num, i, j;
	RECT          rc;
	p_LAB_RECWORD p_rec;
	char          buff[LAB_RW_SIZE];

	hDC = GetDC(hWnd);
	GetClientRect(hWnd, &rc);
	rc.right -= DLG_OFFSET;
	rc.left += DLG_OFFSET;
	rc.bottom -= DLG_OFFSET;
	rc.top += DLG_OFFSET;
	word_num = lpAnswers->word_index >= 0 ? lpAnswers->word_index : HIWORD(line_col);
	letter_num = LOWORD(line_col);
	//  p_rec = (p_LAB_RECWORD)&(lpAnswers->rec_words);
	p_rec = (p_LAB_RECWORD) &(lpAnswers->rec_words[0]);
	p_rec += word_num;
	if (letter_num == 0 || letter_num / 2 - 1 > lstrlen(p_rec->rwword))
	{
		//insert letter
		buff[0] = Answer;
		buff[1] = 0;
		if (lstrlen(p_rec->rwword) == LAB_RW_SIZE - 1)
		{
			p_rec->rwword[LAB_RW_SIZE - 2] = 0;
		}
		if (letter_num == 0)
		{
			lstrcat(buff, p_rec->rwword);
			lstrcpy(p_rec->rwword, buff);
		}
		else
		{
			lstrcat(p_rec->rwword, buff);
		}
		*bChangeFont = TRUE;
	}
	else
		if (!(letter_num & 1))
		{
			//insert letter
			letter_num = letter_num / 2 - 1;
			if (lstrlen(p_rec->rwword) == LAB_RW_SIZE - 1)
			{
				p_rec->rwword[LAB_RW_SIZE - 1] = 0;
			}
			j = 0;
			for (i = 0; i < lstrlen(p_rec->rwword); i++)
			{
				buff[j] = p_rec->rwword[i];
				if (letter_num == i)
				{
					j++;
					buff[j] = Answer;
				}
				j++;
			}
			buff[j] = 0;
			lstrcpy(p_rec->rwword, buff);
			*bChangeFont = TRUE;
		}
		else
		{
			//overwrite letter
			p_rec->rwword[letter_num / 2] = Answer;
			*bChangeFont = FALSE;
		}
	ReleaseDC(hWnd, hDC);
	InvalidateRect(hWnd, NULL, TRUE);
	UpdateWindow(hWnd);
	lpAnswers->ilast = word_num;
	lstrcpy(lpAnswers->last, p_rec->rwword);
	if (lpAnswers->word_index >= 0)
	{
		SaveCorrection(GetDlgItem(hLastDebugWnd, DRAWINK), lpAnswers->ilast, lpAnswers->last);
	}
} /* end of DrawResult */

/******************************************************************************************/
static void DrawLetter(HDC hDC, p_CORANSW_TYPE lpAnswers, LPRECT lprc, int line, int letter)
{
	int           x, y;
	p_LAB_RECWORD p_rec;
	TEXTMETRIC    tm;
	SIZE          size;

	//  p_rec = (p_LAB_RECWORD)&(lpAnswers->rec_words);
	p_rec = (p_LAB_RECWORD) &(lpAnswers->rec_words[0]);
	p_rec += lpAnswers->word_index >= 0 ? lpAnswers->word_index : line;
	GetTextMetrics(hDC, &tm);
	GetTextExtentPoint(hDC, &p_rec->rwword[letter], 1, &size);
	x = lprc->left + (letter * 2 + 1)*lpAnswers->guide.cxBox +
	    (lpAnswers->guide.cxBox - size.cx) / 2;
	y = lprc->top + line*lpAnswers->guide.cyBox + lpAnswers->guide.cyBase - tm.tmAscent;
	TextOut(hDC, x, y, &p_rec->rwword[letter], 1);
} /* end of DrawLetter */

/******************************************************************************************/
static void DrawGuidesAndAnswers(HDC hDC, p_CORANSW_TYPE lpAnswers,
                                 LPRECT lprc, int word_number)
{
	int           i;
	p_LAB_RECWORD p_rec;
	HPEN          hPen, hOldPen;
	COLORREF      colorOld;

	//  p_rec = (p_LAB_RECWORD )&(lpAnswers->rec_words);
	p_rec = (p_LAB_RECWORD) &(lpAnswers->rec_words[0]);
	p_rec += lpAnswers->word_index >= 0 ? lpAnswers->word_index : word_number;
	hPen = CreatePen(PS_DOT, 1, RGB(0, 0, 0));
	hOldPen = (HPEN) SelectObject(hDC, hPen);
	// draw mid line
	MoveToEx(hDC, lprc->left, lprc->top +
	         word_number*lpAnswers->guide.cyBox + lpAnswers->guide.cyBase -
	         lpAnswers->guide.cyMid, NULL);
	LineTo(hDC, lprc->right, lprc->top +
	       word_number*lpAnswers->guide.cyBox + lpAnswers->guide.cyBase -
	       lpAnswers->guide.cyMid);
	SelectObject(hDC, hOldPen);
	if (hPen)
	{
		DeleteObject(hPen);
	}
	// draw base line
	MoveToEx(hDC, lprc->left, lprc->top +
	         word_number*lpAnswers->guide.cyBox + lpAnswers->guide.cyBase, NULL);
	LineTo(hDC, lprc->right, lprc->top +
	       word_number*lpAnswers->guide.cyBox + lpAnswers->guide.cyBase);
	// draw letters
	colorOld = SetTextColor(hDC, RGB(144, 144, 144));
	for (i = 0; i < lstrlen(p_rec->rwword); i++)
	{
		DrawLetter(hDC, lpAnswers, lprc, word_number, i);
	}
	SetTextColor(hDC, colorOld);
} /* end of DrawGuidesAndAnswers */

/******************************************************************************************/
LRESULT CALLBACK hlvTrajProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT       ps;
	HDC               hDC;
	HGLOBAL           hHlv;
	LP_HLV            lpHlv;
	LP_HLV_SECT       lpSect;
	HWND              hInkWnd;
	RECT              rc, rcTrace;
	GLB_DATA_TYPE     glb;
	WORD              s_beg, s_end, i;
	LP_TRACE          lpTrace; // = &(lpSect->trace); //CHE: 12-27-94

	hInkWnd = GetDlgItem(hLastDebugWnd, DRAWINK);
	hHlv = (HGLOBAL) GetWindowLong(hInkWnd, GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);
	if (lpHlv == NULL)
	{
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	if (lpHlv->lpCurSect == NULL)
	{
		goto Unlock;
	}
	lpSect = lpHlv->lpCurSect;
	switch (message)
	{
		case  WM_PAINT:
			if (lpSect->trace.nPoints > 0)
			{
				GetClientRect(hWnd, &rc);
				hDC = BeginPaint(hWnd, &ps);
				trcMeasureWord(&(lpSect->trace), 0, &rcTrace);
				InflateRect(&rc, -2, -2);
				glbWindow(&glb, &rcTrace);
				glbViewport(&glb, &rc);
				lpTrace = &(lpSect->trace);
				if (lpTrace->lpiStrokes)
				{
					for (i = 0; i < (UINT) lpTrace->nStrokes; i++)
					{
						s_beg = lpTrace->lpiStrokes[i];
						s_end = lpTrace->lpiStrokes[i + 1];
						glbPolyline(&glb, hDC, &lpTrace->lppPoints[s_beg], s_end - s_beg);
					}
				}
				EndPaint(hWnd, &ps);
			}
			break;
		default:
			break;
	}
Unlock:
	DebugUnlockHandle(hHlv);
	return DefWindowProc(hWnd, message, wParam, lParam);
} /* end of hlvTrajProc */

/******************************************************************************************/
static BOOL SetWindowsData(HWND hWnd, WORD WindowID, LP_HLV_SECT lpSect,
                           BOOL Reset, BOOL Delete)
{
	HWND            CtlWnd;
	GLOBALHANDLE    hData;
	int             size;
	p_CORANSW_TYPE   lpAnswers;

	CtlWnd = GetDlgItem(hWnd, WindowID);
	if (CtlWnd == NULL)
	{
		return TRUE;
	}
	hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
	if (Reset)
	{
		DebugFree(hData, "WGHLV SetWindowsData");
		if (Delete)
		{
			return TRUE;
		}
	}
	if (WindowID == ID_LETTERS)
	{
		size = sizeof(CORANSW_TYPE);
	}
	hData = DebugAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, size, "WGHLV SetWindowsData");
	if (hData == NULL)
	{
		SetWindowLong(CtlWnd, GWW_PRIVATE_DATA, (LONG) hData);
		return FALSE;
	}
	if (WindowID == ID_LETTERS)
	{
		lpAnswers = (p_CORANSW_TYPE) DebugLockHandle(hData);
		lpAnswers->n_rec_words = lpSect->n_rec_words;
		_fmemcpy((p_LAB_RECWORD) &lpAnswers->rec_words[0],
		         lpSect->rec_words, sizeof(LAB_RECWORD_TYPE)*lpSect->n_rec_words);
		lpAnswers->word_index = lpSect->selcor;
	}
	DebugUnlockHandle(hData);
	SetWindowLong(CtlWnd, GWW_PRIVATE_DATA, (LONG) hData);
	return TRUE;
	return FALSE;
} /* end of SetWindowsData */

/******************************************************************************************/
LRESULT CALLBACK hlvAnswersDlgBox(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static LP_HLV_SECT lpSect;
	HGLOBAL            hData;
	p_CORANSW_TYPE     lpAnswers;
	static int NumPntsSav;
	static LPPOINT pTraceSav;

	switch (message)
	{
		case WM_INITDIALOG:
			bFirstStroke = TRUE;
			lpSect = (LP_HLV_SECT) lParam;
			NumPntsSav = lpSect->np_lo_trace;
			pTraceSav = lpSect->lo_trace;
			lpSect->np_lo_trace = 0;
			lpSect->lo_trace = (LPPOINT) DebugAllocPtr(GSHND, LAB_MAX_TRACE*sizeof(POINT), "WGHLV hlvAnswersDlgBox");
			if (lpSect->lo_trace == _NULL)
			{
				MessageBox(hDlg, "No memory for trace", "Corrector", MB_OK);
				PostMessage(hDlg, WM_COMMAND, IDCANCEL, lParam);
			}
			else
			{
				SetWindowsData(hDlg, ID_LETTERS, lpSect, FALSE, FALSE);
			}
			return TRUE;

		case WM_COMMAND:
			switch (GET_WM_COMMAND_ID(wParam, lParam))
			{
				case IDOK:
					hData = (HGLOBAL) GetWindowLong(GetDlgItem(hDlg, ID_LETTERS), GWW_PRIVATE_DATA);
					lpAnswers = (p_CORANSW_TYPE) DebugLockHandle(hData);
					if (lpAnswers != NULL && lpAnswers->last[0])
					{
						SaveCorrection(GetDlgItem(hLastDebugWnd, DRAWINK), lpAnswers->ilast, lpAnswers->last);
					}
					DebugUnlockHandle(hData);
					{
						extern _UCHAR fl_cut;
						p_rec_info_type pri = recGetRCinfo();
						int i;
						PS_point_type *pTr = (PS_point_type *) pri->pTrace;
						RECT rWrWnd;
						for (i = 0; i < lpSect->np_lo_trace; i++)
						{
							pTr->x = lpSect->lo_trace[i].x;
							pTr->y = lpSect->lo_trace[i].y;
							pTr++;
						}
						GetClientRect(GetDlgItem(hDlg, ID_LETTERS), &rWrWnd);
						pri->bb_bottom = rWrWnd.top + lpAnswers->guide.cyBase;
						pri->bb_top = pri->bb_bottom - lpAnswers->guide.cyMid;
						pri->fl_bedit_border = 1;
						//CHE ((p_rc_type)(pri->prc))->low_mode=LMOD_BOX_EDIT;
						fl_cut = pri->fl_cut = 0;
						pri->num_points_got = lpSect->np_lo_trace;
						DebugFreePtr(pTraceSav, "WGHLV hlvAnswersDlgBox");
						hAnswerDlgBox = NULL;
						hCorrectDlgBox = NULL;
						if (lpSect->np_lo_trace > 5) //CHE
						{
							DoRecognition(pri);
						}
						EndDialog(hDlg, wParam == IDOK);
						return TRUE;
					}
				case IDCANCEL:
					lpSect->np_lo_trace = NumPntsSav;
					if (lpSect->lo_trace != _NULL)
					{
						DebugFreePtr(lpSect->lo_trace, "WGHLV hlvAnswersDlgBox");
					}
					lpSect->lo_trace = pTraceSav;
					hAnswerDlgBox = NULL;
					hCorrectDlgBox = NULL;
					EndDialog(hDlg, wParam == IDCANCEL);
					return TRUE;
			}
			break;

	}
	return FALSE;
} /* end of hlvAnswersDlgBox */

/******************************************************************************************/
static void DrawButtons(HDC hDC, LPRECT rc, LONG Id)
{
	HBITMAP hToolBitMap = NULL, hOld;
	HDC     hDCMem;
	WORD    BitMapID;

	//BitMapID = (Id == ID_EXIT) ? TOOLCLOSEWINDOW : TOOLMOVEWINDOW;
	BitMapID = (Id == ID_EXIT) ? TOOLCORRECT_OK : TOOLCORRECT_CANCEL;
	hToolBitMap = LoadBitmap(hInst, MAKEINTRESOURCE(TOOLBMP));
	hDCMem = CreateCompatibleDC(hDC);
	hOld = (HBITMAP) SelectObject(hDCMem, hToolBitMap);
	BitBlt(hDC, rc->left, rc->top, 16, 16,
	       hDCMem, rc->left + (BitMapID % 16) * 16, rc->top + (BitMapID / 16) * 16,
	       SRCCOPY);
	SelectObject(hDCMem, hOld);
	if (hToolBitMap)
	{
		DeleteObject(hToolBitMap);
	}
	DeleteDC(hDCMem);
} /* end of DrawButtons */

/******************************************************************************************/
LRESULT  CALLBACK hlvCorToolsProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT              rc;
	HDC               hDC;
	PAINTSTRUCT       ps;
	LONG              Id;
	static RECT       oldrect;
	static POINT      pt;
	static POINT      pt_corner;

#ifdef _WIN32
	Id = GetWindowLong(hWnd, GWL_ID);
#else
	Id = (LONG)GetWindowWord(hWnd, GWW_ID);
#endif
	switch (message)
	{
		case WM_LBUTTONDOWN:
			if (Id == ID_MOVE)
			{
				//PostMessage(GetParent(hWnd), WM_COMMAND, IDCANCEL, 0L);
				/*        SetCapture(hWnd);
						GetWindowRect(hMainWnd, &rcMain);
						ClipCursor(&rcMain);
						pt.x = LOWORD(lParam);
						pt.y = HIWORD(lParam);
						hDC = GetDC(hWnd);
						LPtoDP(hDC, &pt, 1);
						GetWindowRect(GetParent(hWnd), &oldrect);
						pt_corner.x = oldrect.left;
						pt_corner.y = oldrect.top;
						LPtoDP(hDC, &pt_corner, 1);
						ReleaseDC(hWnd, hDC);
						*/
			}
			return 0;

		case WM_LBUTTONUP:
			if (Id == ID_EXIT)
			{
				PostMessage(GetParent(hWnd), WM_COMMAND, IDOK, 0L);
			}
			else
				if (Id == ID_MOVE)
				{
					PostMessage(GetParent(hWnd), WM_COMMAND, IDCANCEL, 0L);
				}
			return 0;

		/*    case WM_LBUTTONUP   :
			  if (Id == ID_MOVE) {
			  ClipCursor(NULL);
			  ReleaseCapture();
			  pt1.x = LOWORD(lParam);
			  pt1.y = HIWORD(lParam);
			  hDC = GetDC(hWnd);
			  LPtoDP(hDC, &pt1, 1);
			  GetWindowRect(GetParent(hWnd), &oldrect);
			  oldrect.left   += pt1.x - pt.x;
			  oldrect.top    += pt1.y - pt.y;
			  DPtoLP(hDC, &pt, 1);
			  ReleaseDC(hWnd, hDC);
			  SetWindowPos(GetParent(hWnd), (HWND)NULL, oldrect.left, oldrect.top,
			  0, 0, SWP_NOSIZE);
			  }
			  return 0;
			  */
		case  WM_PAINT:
			hDC = BeginPaint(hWnd, &ps);
			GetClientRect(hWnd, &rc);
#ifdef _WIN32
			Id = GetWindowLong(hWnd, GWL_ID);
#else
			Id = (LONG)GetWindowWord(hWnd, GWW_ID);
#endif
			DrawButtons(hDC, &rc, Id);
			EndPaint(hWnd, &ps);
			return 0;

		default:
			break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
} /* end of hlvCorToolsProc */

/******************************************************************************************/
/******************************************************************************************/
/*******************************  COMPRESS UTILITIES  *************************************/
/******************************************************************************************/
/******************************************************************************************/

static void CompressTrajectory(LP_HLV_SECT lpSect, WPARAM wParam)
{
	//??SD for the time being is not implied
	return;
} /* end of CompressTrajectory */


/*****************************************************************************/
static void ShowXRInput(LP_HLV_SECT lpSect, int xrnumber, POINT pt)
{
	XRINPUTSHOW_TYPE xr_inp;

	if (lpSect->xrs == NULL)
	{
		return;
	}

	xr_inp.version = GetDTIVersionForPenlab(recGetRCinfo());
	if (xr_inp.version == -1)
	{
		return;
	}
	xr_inp.xrnumber = xrnumber;
	xr_inp.pxr_input = lpSect->xrs;
	xr_inp.pxr_ext = NULL;
	hCorResult = CreateDialogParam(hInst, "XRINPUT", hLastDebugWnd,
	                               (DLGPROC) hlvInputResult, (LPARAM) &xr_inp);

} /* end of ShowXRInput */

/*****************************************************************************/
static void ShowXRCorrelation(LP_HLV_SECT lpSect, int xrnumber, POINT pt)
{
	READMATRIX_TYPE rm;

	if (lpSect->ppd == NULL || lpSect->xrs == NULL)
	{
		return;
	}

	rm.pri = recGetRCinfo();
	if (rm.pri == NULL)
	{
		return;
	}
	rm.ppd = lpSect->ppd;
	rm.nxr = xrnumber;
	rm.pxr_input = lpSect->xrs;
	SetDebugMode(HWDBG_GETMATRIX, (LPARAM) (p_VOID) &rm);
	if (rm.nxr != READ_MATRIX_ERROR)
	{
		if (rm.version == 0)
		{
			hCorResult = CreateDialogParam(hInst, "CORRESULT", hLastDebugWnd,
			                               (DLGPROC) hlvCorResult, (LPARAM) &rm);
		}
		else
		{
			hCorResult = CreateDialogParam(hInst, "CORRESULT2", hLastDebugWnd,
			                               (DLGPROC) hlvCorResult, (LPARAM) &rm);
		}
	}
} /* end of ShowXRCorrelation */

/*****************************************************************************/
LRESULT  CALLBACK hlvCorResult(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	p_READMATRIX p;
	char         buff[24];

	if (message == WM_INITDIALOG)
	{
		p = (p_READMATRIX) lParam;

		if (p->version == 0) // Standart version (3 param)
		{
			wsprintf(buff, "var = %d", p->nVar);
			SetDlgItemText(hWnd, ID_NVAR, buff);
			wsprintf(buff, "Cxr = %d", p->Cxr);
			SetDlgItemText(hWnd, ID_CXR, buff);
			wsprintf(buff, "Ch = %d", p->Cheight);
			SetDlgItemText(hWnd, ID_CH, buff);
			wsprintf(buff, "Csh = %d", p->Cshift);
			SetDlgItemText(hWnd, ID_CSH, buff);
			wsprintf(buff, "Pv = %d", p->Cpenalty);
			SetDlgItemText(hWnd, ID_PV, buff);
			wsprintf(buff, "Pi = %d", p->Pi);
			SetDlgItemText(hWnd, ID_PI, buff);
		}
		else // 5-param version
		{
			wsprintf(buff, "var = %d", p->nVar);
			SetDlgItemText(hWnd, ID_NVAR, buff);
			wsprintf(buff, "Cxr = %d", p->Cxr);
			SetDlgItemText(hWnd, ID_CXR, buff);
			wsprintf(buff, "Ch = %d", p->Cheight);
			SetDlgItemText(hWnd, ID_CH, buff);
			wsprintf(buff, "Csh = %d", p->Cshift);
			SetDlgItemText(hWnd, ID_CSH, buff);
			wsprintf(buff, "Pv = %d", p->Cpenalty);
			SetDlgItemText(hWnd, ID_PV, buff);
			wsprintf(buff, "Pi = %d", p->Pi);
			SetDlgItemText(hWnd, ID_PI, buff);
			wsprintf(buff, "Co = %d", p->Corient);
			SetDlgItemText(hWnd, ID_OR, buff);
			wsprintf(buff, "Cl = %d", p->Cdepth);
			SetDlgItemText(hWnd, ID_LI, buff);
			wsprintf(buff, "Attr = %x", p->Cattrib);
			SetDlgItemText(hWnd, ID_ATTR, buff);
		}

		return 0L;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
} /* end of hlvCorResult */

/*****************************************************************************/
LRESULT  CALLBACK hlvInputResult(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	char             buff[24];
	XRINPUTSHOW_TYPE *p;
	LAB_XRNODE_TYPE        xrd;

	p = (XRINPUTSHOW_TYPE *) lParam;
	/*
	  p->xrnumber;
	  p->pxr_input = lpSect->xrs;
	  p->pxr_ext = NULL;
	  */
	if (message == WM_INITDIALOG)
	{
		if (p->version == 0)
		{
			xrd = p->pxr_input[p->xrnumber].xdxr;
			wsprintf(buff, "xr = %d", xrd.xnxr);
			SetDlgItemText(hWnd, ID_XRNUMBER, buff);
			wsprintf(buff, "h = %d", xrd.xnh & 0x0f);
			SetDlgItemText(hWnd, ID_XRHEIGHT, buff);
			wsprintf(buff, "s = %d", xrd.xnh >> 4);
			SetDlgItemText(hWnd, ID_XRSHIFT, buff);
			wsprintf(buff, "p = %d", xrd.xnp);
			SetDlgItemText(hWnd, ID_XRPENALTY, buff);
			wsprintf(buff, "a = %x", xrd.xna);
			SetDlgItemText(hWnd, ID_XRATTR, buff);
		}
		else    //5 param
		{
			xrd = p->pxr_input[p->xrnumber].xdxr;
			wsprintf(buff, "xr = %d", xrd.xnxr);
			SetDlgItemText(hWnd, ID_XRNUMBER, buff);
			wsprintf(buff, "h = %d", xrd.xnh);
			SetDlgItemText(hWnd, ID_XRHEIGHT, buff);
			wsprintf(buff, "s = %d", xrd.xns);
			SetDlgItemText(hWnd, ID_XRSHIFT, buff);
			wsprintf(buff, "o = %d", xrd.xno);
			SetDlgItemText(hWnd, ID_XRORIENT, buff);
			wsprintf(buff, "l = %d", xrd.xnd);
			SetDlgItemText(hWnd, ID_XRLINK, buff);
			wsprintf(buff, "p = %d", xrd.xnp);
			SetDlgItemText(hWnd, ID_XRPENALTY, buff);
			wsprintf(buff, "a = %x", xrd.xna);
			SetDlgItemText(hWnd, ID_XRATTR, buff);
		}

		return 0L;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
} /* end of hlvInputResult */

/*****************************************************************************/
void FAR hlvKillCorrelationResultDialog(void)
{
	if (hCorResult != 0)
	{
		DestroyWindow(hCorResult);
		hCorResult = NULL;
		SetFocus(hLastDebugWnd);
	}
} /* end of hlvKillCorrelationResultDialog */

/*****************************************************************************/
BOOL FAR hlvDDEData(LPARAM pXrd)
{
	HGLOBAL     hHlv;
	HWND        hWnd;
	LP_HLV      lpHlv;
	LP_HLV_SECT lpSect;
	BOOL        bRet = FALSE;
	_INT        n;

	if (hLastDebugWnd == NULL)
	{
		goto ret;
	}
	hWnd = GetDlgItem(hLastDebugWnd, DRAWINK);
	hHlv = (HGLOBAL) GetWindowLong(hWnd, GWW_HLV_DATA);
	lpHlv = (LP_HLV) DebugLockHandle(hHlv);
	if (lpHlv == NULL)
	{
		goto ret;
	}
	lpSect = lpHlv->lpCurSect;
	if (lpSect == NULL)
	{
		goto ret;
	}
	for (n = 0; n < lpSect->n_xrs && lpSect->xrs[n].xdxr.xnxr != 0; n++)
		if (lpSect->xrs[n].xdxr.xnxr != ((xrvoc_type *) pXrd)[n].xr
		        || lpSect->xrs[n].xdxr.xna != ((xrvoc_type *) pXrd)[n].a
		        || lpSect->xrs[n].xdxr.xnp != ((xrvoc_type *) pXrd)[n].p
		        || lpSect->xrs[n].xdxr.xnh != ((xrvoc_type *) pXrd)[n].h
		   )
		{
			lpSect->IndexDiffXr = n;
			SetFocus(hLastDebugWnd);
			InvalidateRect(hWnd, &(lpSect->pxrrect_input[n]), TRUE);
			UpdateWindow(hWnd);
			break;
		}

ret:
	DebugUnlockHandle(hHlv);
	return bRet;

} /* end of hlvDDEData */
