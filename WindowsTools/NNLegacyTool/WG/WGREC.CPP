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
#ifndef _PENWIN
#include "pensub.h32"
#else
#include <penwin.h>
#include <penwoem.h>
#include <hwr_sys.h>
#include <ams_mg.h>
#include <xrword.h>
#include <learn.h>
#endif
#include <direct.h>
#include <io.h>
#include <string.h>
#include <stdlib.h>
#include <commdlg.h>
#include <limits.h>
#include <bastypes.h>
#include <wg_stuff.h>
#include "wggbl.h"
#include "wgidm.h"
#include "wgdbo.h"
#include "wgmsg.h"
#include "wgtrc.h"
#include "wghlv.h"
#include "wgtls.h"
#include "wgirc.h"
#include "wgtap.h"
#include "wgdes.h"
#include "wgprf.h"
#include "wgfak.h"
#include "wgsta.h"
#include "wgpse.h"
#include "wgsrl.h"
#include "wgdbg.h"
#include "wgtxt.h"
#include "wgrec.h"

extern dbgSetDebugMode_TYPE SetDebugMode;
#define DebugHole SetDebugMode


#ifndef N_WORD
#define N_WORD 154
#endif

#ifndef PS_PSINI_CHANGE
#define PS_PSINI_CHANGE 4
#endif

#ifndef PS_RELOAD_DAT
#define PS_RELOAD_DAT   7
#endif

#ifndef PS_SERV_DB
#define PS_SERV_DB 6
#endif

#ifdef _PENWIN
HPENDATA GlueTracePenData(LP_TRACE lpTrace, LPSTR lpSel);
HREC hrecParaPen = NULL;
#else
typedef int (WINAPI *INITREC)(LPSTR, p_rec_info_type);
typedef int (WINAPI *CLOSEREC)(p_rec_info_type);
typedef int (WINAPI *RECOGNIZE)(p_rec_info_type);
p_rec_info_type lpRCinfo;
HINSTANCE   hrecParaPen = NULL;
static int  FindRecNameInIniFile(LPSTR TempRecognizerName);
static int  AddTracePenDataNT(LP_TRACE lpTrace, p_TRACEDATA lpTempTrace,
                              int nWord, BOOL LastWord);
static int  SetRCinfoTrace(p_TRACEDATA lpTempTrace);
#endif

char      zRecognizerPathName[_MAX_PATH];
char      zRecognizerFakeName[_MAX_PATH];
char      zPsIniName[_MAX_PATH] = "";
LPGUIDE   glpGuides = NULL;
BOOL      bUseFake = TRUE;
FARPROC   lpfnLoadRec;


static    HCURSOR   hLocalCur, hLocalWaitCur;
/*******************************************************************/
#ifndef _PENWIN
p_rec_info_type FAR recGetRCinfo(void)
{
	return lpRCinfo;
} /* end of recRCinfo */

/*******************************************************************/
HINSTANCE FAR recGetDLLHandle(void)
{
	return   hrecParaPen;
} /* end of recGetDLLHandle */

/*******************************************************************/
static int SetRCinfoTrace(p_TRACEDATA lpTempTrace)
{
	RCBACCESS       lpfnRC;
	int             result;

	RETURN_IF_BAD_POINTER(hrecParaPen);
	RETURN_IF_BAD_POINTER(lpRCinfo);
	lpfnRC = (RCBACCESS) Q_GetProcAddress(hrecParaPen, REC_DATA_ACCESS);
	RETURN_IF_BAD_POINTER(lpfnRC);

	// set recognizer trace
	result = (*lpfnRC)((void FAR *)lpRCinfo, OBJ_TRAJECTORY, CMD_SET,
	                   (UINT) lpTempTrace->TDnp, (ULONG) lpTempTrace->TDpt);
	return result;
} /* end of SetRCinfoTrace */

/*******************************************************************/
static int AddTracePenDataNT(LP_TRACE lpTrace, p_TRACEDATA lpTempTrace,
                             int nWord, BOOL LastWord)
{
	int     firstStroke, stopStroke, firstPoint, stopPoint;
	int     i, j;
	static  int stroke_num;
	LPPOINT lppPoints;

	if (hrecParaPen == NULL)
	{
		return FALSE;
	}

	lppPoints = lpTrace->lppPoints;
	firstStroke = lpTrace->lpiWords[nWord];
	stopStroke = lpTrace->lpiWords[nWord + 1];

	if (lpTempTrace->TDnp == 0)
	{
		stroke_num = 0;
		lpTempTrace->TDpt[lpTempTrace->TDnp].x = 0;
		lpTempTrace->TDpt[lpTempTrace->TDnp].y = -1;
		lpTempTrace->TDnp++;
	}
	for (i = firstStroke; i < stopStroke; i++)
	{
		firstPoint = lpTrace->lpiStrokes[i];
		stopPoint = lpTrace->lpiStrokes[i + 1];

		for (j = firstPoint; j < stopPoint; j++)
		{
			if (lpTempTrace->TDnp >= LAB_MAX_TRACE - 32)
			{
				MessageBox(0, "Temp trace is full. \nIncrease LAB_MAX_TRACE.", "int AddTracePenDataNT()", MB_ICONSTOP | MB_OK | MB_SETFOREGROUND);
				goto err;
			}

			lpTempTrace->TDpt[lpTempTrace->TDnp].x = (short) lppPoints[j].x;
			lpTempTrace->TDpt[lpTempTrace->TDnp].y = (short) lppPoints[j].y;
			lpTempTrace->TDnp++;
		}

		stroke_num++;
		lpTempTrace->TDpt[lpTempTrace->TDnp].x = stroke_num;
		lpTempTrace->TDpt[lpTempTrace->TDnp].y = -1;
		lpTempTrace->TDnp++;
	}
	return TRUE;
err:
	return FALSE;
} /* end of AddTracePenDataNT */

/*******************************************************************/
#else
static HPENDATA AddTracePenData(HPENDATA hpendata, LP_TRACE lpTrace,
                                int nWord, BOOL LastWord)
{
	int i;
	int firstStroke, stopStroke, firstPoint, stopPoint;
	STROKEINFO    si;
	LPPOINT       lppPoints = lpTrace->lppPoints;
	int           LastPenUpStroke;
	HPENDATA      hpd;

	si.cbPnts = 0;
	si.dwTick = 0;

	if (hpendata == NULL)
	{
		hpendata = CreatePenData(NULL, 0, PDTS_STANDARDSCALE, GMEM_SHARE);
	}
	firstStroke = lpTrace->lpiWords[nWord];
	stopStroke = lpTrace->lpiWords[nWord + 1];
	if (LastWord)
	{
		LastPenUpStroke = stopStroke - 1;
	}
	else
	{
		LastPenUpStroke = stopStroke;
	}
	for (i = firstStroke; i < stopStroke; i++)
	{
		firstPoint = lpTrace->lpiStrokes[i];
		stopPoint = lpTrace->lpiStrokes[i + 1];
		si.cPnt = stopPoint - firstPoint;
		si.wPdk = PDK_DOWN | PDK_DRIVER;
		hpd = AddPointsPenData(hpendata, &lppPoints[firstPoint], NULL, &si);
		if (hpd == NULL)
		{
			return hpendata;
		}
		else
		{
			hpendata = hpd;
		}
		if (i < LastPenUpStroke)
		{
			si.cPnt = 1;
			si.wPdk = PDK_UP | PDK_DRIVER;
			hpd = AddPointsPenData(hpendata, &lppPoints[stopPoint - 1], NULL, &si);
			if (hpd == NULL)
			{
				return hpendata;
			}
			else
			{
				hpendata = hpd;
			}
		}
	}
	return hpendata;
} /* AddTracePenData */
#endif

/*******************************************************************/
#ifndef _PENWIN
static int  GlueTracePenDataNT(LP_TRACE lpTrace, LPSTR lpSel)
{
	int            i, j;
	BOOL           fAtLeastOne;
	TRACEDATA_TYPE TempTrace;

	fAtLeastOne = FALSE;
	if (lpSel != NULL)
	{
		for (i = 0; i < lpTrace->nWords; i++)
		{
			if (lpSel[i])
			{
				fAtLeastOne = TRUE;
				break;
			}
		}
	}
	j = lpTrace->nWords - 1;
	TempTrace.TDnp = 0;
	TempTrace.TDpt =
	    (LPPOINT) DebugAllocPtr(GHND, sizeof(POINT)*LAB_MAX_TRACE, "WGREC GlueTracePenDataNT");
	RETURN_IF_BAD_POINTER(TempTrace.TDpt);
	for (i = 0; i < lpTrace->nWords; i++)
	{
		if (fAtLeastOne && !lpSel[i])
		{
			continue;
		}
		AddTracePenDataNT(lpTrace, &TempTrace, i, i == lpTrace->nWords - 1);
	}
	SetRCinfoTrace(&TempTrace);
	DISPOSE(TempTrace.TDpt, "WGREC GlueTracePenDataNT");
	return TRUE;
} /* end of GlueTracePenDataNT */
/*******************************************************************/

#else
HPENDATA GlueTracePenData(LP_TRACE lpTrace, LPSTR lpSel)
{
	int i, j;
	HPENDATA hpendata;
	BOOL fAtLeastOne;

	hpendata = NULL;
	fAtLeastOne = FALSE;
	if (lpSel != NULL)
	{
		for (i = 0; i < lpTrace->nWords; i++)
		{
			if (lpSel[i])
			{
				fAtLeastOne = TRUE;
				break;
			}
		}
	}
	j = lpTrace->nWords - 1;
	if (fAtLeastOne)
		for (i = 0; i < lpTrace->nWords; i++)
			if (lpSel[i])
			{
				j = i;
			}
	for (i = 0; i < lpTrace->nWords; i++)
	{
		if (fAtLeastOne && !lpSel[i])
		{
			continue;
		}
		hpendata = AddTracePenData(hpendata, lpTrace, i, i == j);
	}
	return hpendata;
} /* GlueTracePenData */
#endif

/************************************************************************************/
BOOL FAR recRecognizeTrace(LP_TRACE lpTrace, LPSTR lpSel, BOOL UseUpper, BOOL WordCut, int WordOffset)
{
#ifdef _PENWIN
	HPENDATA        hpendata;
	GUIDE           guide;
#else
	TRACEDATA_TYPE  TempTrace;
#endif
	int             i;
	HWND            hWndDebug;
	BOOL            fAtLeastOne, result;

	msgHWDebug(hWndClient, WM_HWDBG, 0, 0L);
	hWndDebug = GetDlgItem(hLastDebugWnd, DRAWINK);
#ifdef _PENWIN
	glpGuides = NULL;
#else
	if (lpRCinfo == NULL)
	{
		// no installed recognizer
		result = recLoadRecognizerNT(NULL);
		RETURN_IF_BAD_RESULT(result, 0);
	}
	// GIT & SD - reset fields in RCinfo, which contain up-down baseline values
	SetDebugMode(HWDBG_NEWFILE, (LPARAM) lpRCinfo);
#endif

	if (!gWordCut)          /* if recognizer itself is in WordCut mode we must also be */
		if (msgGetWordCut())
		{
			gWordCut = TRUE;
		}
	// find first selected word on the page
	if (lpSel != NULL)
	{
		// recognize request from TAP
		lpTrace->FirstWord = 0;
		for (i = 0; i < lpTrace->nWords; i++)
		{
			if (lpSel[i])
			{
				lpTrace->FirstWord = i;
				break;
			}
		}
	}
	else
	{
		// on-line recognition request
	}
	lpTrace->CurrentStrokeBegin = lpTrace->lpWordStrokes[lpTrace->FirstWord].begin;

	if (gWordCut)
	{
		/* Glue up all the trajectory in one PenData */
		if (lpSel != NULL)
		{
			hlvSetCurSect(lpTrace->FirstWord, 0);
		}
		else /* on-line call */
		{
			hlvSetCurSect(-1, 0);
		}
#ifdef _PENWIN
		hpendata = GlueTracePenData(lpTrace, lpSel);
		recRecognizeData(hWndDebug, hpendata, glpGuides, UseUpper, TRUE);
		DestroyPenData(hpendata);
#else
		GlueTracePenDataNT(lpTrace, lpSel);
		recRecognizeDataNT(hWndDebug, UseUpper, TRUE);
#endif
	}
	else
	{
		/* Feed the recognizer with each word separately */
		fAtLeastOne = FALSE;
		if (lpSel != NULL)
		{
			for (i = 0; i < lpTrace->nWords; i++)
			{
				if (lpSel[i])
				{
					fAtLeastOne = TRUE;
					break;
				}
			}
		}
		for (i = 0; i < lpTrace->nWords; i++)
		{
			if (fAtLeastOne && !lpSel[i])
			{
				continue;
			}
#ifdef _PENWIN
			hpendata = AddTracePenData(NULL, lpTrace, i, TRUE);
			glpGuides = &(lpTrace->lpgWords[i]);
			if (glpGuides->cxBox != 0 && glpGuides->cyBox != 0)
			{
				/* Trust me, we REALLY need to save the guide somewhere */
				_fmemcpy(&guide, glpGuides, sizeof(GUIDE));
				glpGuides = &guide;
			}
			else
			{
				glpGuides = NULL;
			}
#else
			TempTrace.TDnp = 0;
			TempTrace.TDpt =
			    (LPPOINT) DebugAllocPtr(GHND, sizeof(POINT)*LAB_MAX_TRACE, "WGREC recRecognizeTrace");
			RETURN_IF_BAD_POINTER(TempTrace.TDpt);
			result = AddTracePenDataNT(lpTrace, &TempTrace, i, TRUE);
			RETURN_IF_BAD_RESULT(result, 0);
			SetRCinfoTrace(&TempTrace);
			DISPOSE(TempTrace.TDpt, "WGREC recRecognizeTrace");
#endif
			if (lpSel == NULL) /* on-line call */
			{
				hlvSetCurSect(-1, 0);
			}
			else
			{
				hlvSetCurSect(i, 0);
			}
#ifdef _PENWIN
			recRecognizeData(hWndDebug, hpendata, glpGuides, UseUpper, TRUE);
			DestroyPenData(hpendata);
#else
			tapSetCurrentWord(i); // + WordOffset);
			txtSetNatashaData(i); // + WordOffset);
			lpRCinfo->num_word = (i); // + WordOffset;

			if (lpSel != NULL)
			{
				//??SD 09/26 add
				{
					RCBACCESS         lpfnRC;
					HINSTANCE         hDLL;
					short             CorrectorData[3];
					hDLL = recGetDLLHandle();
					lpfnRC = (RCBACCESS) Q_GetProcAddress(hDLL, REC_DATA_ACCESS);
					if (tapGetCorrectionData(sizeof(CorrectorData) / sizeof(CorrectorData[0]),
					                         &CorrectorData[0]))
					{
						(*lpfnRC)((void FAR *)lpRCinfo,
						          OBJ_RCCORRECTORDATA, CMD_SET,
						          (UINT)sizeof(CorrectorData) / sizeof(CorrectorData[0]),
						          (ULONG) &CorrectorData[0]);
					}
				}
			}
			recRecognizeDataNT(hWndDebug, UseUpper, TRUE);
			//??SD 09/26 end
#endif
			//DebugFree(hpendata, "????");
			/*
			* We need to test IsWindow(hWndDebug) because it may be
			* destroyed in the middle of debugging process. After destroying
			* lpTrace becames invalid.
			*/
			if (!IsWindow(hWndDebug))
			{
				break;
			}
		}
	}
	hlvSetCurSect(-1, 0); // recognition done
	return TRUE;
} /* recRecognizeTrace */

/*******************************************************************************/
#ifndef _PENWIN
BOOL FAR recRecognizeDataNT(HWND hWnd, BOOL UseUpperLevel, BOOL TabletCoord)
{
	short     OldDebugLevel;
	RECOGNIZE lpfnDoRecognition;
	int       result;

	staSetStatus(ST_RECOGNIZING, (LPARAM) TRUE);
	staSetStatus(ST_FIRSTWORDFROMREC, (LPARAM) TRUE);
	staSetStatus(ST_MAIL, (LPARAM) FALSE);
	tlsGetLevels();               /* ??AS */
	if (UseUpperLevel)
	{
		gUseUpper = TRUE;
		OldDebugLevel = DebugLevel; // run Upper regardless of DebugLevel
		DebugLevel = DEF_DEBUGLEVEL;           //??????SD -10 ??
	}
	tlsUpdateStatusBar(BUSY);
	msgStartDebug();
	result = 0;
	lpfnDoRecognition = (RECOGNIZE) Q_GetProcAddress(hrecParaPen, REC_RECOGNIZE);
	if (lpfnDoRecognition != NULL && lpRCinfo != NULL)
	{
		gIsRecognizing = TRUE;  //CHE
		result = (*lpfnDoRecognition)(lpRCinfo);
		gIsRecognizing = FALSE;  //CHE
	}
	if (UseUpperLevel)
	{
		DebugLevel = OldDebugLevel;
		gUseUpper = FALSE;
		msgUpdateLevels();
	}
	tlsUpdateStatusBar(IDLE);
	gWordCut = FALSE; /* return to default */

	staSetStatus(ST_RECOGNIZING, (LPARAM) FALSE);
	staSetStatus(ST_MAIL, (LPARAM) TRUE);
	return (result == TRUE);
} /* end of recRecognizeDataNT */

/*******************************************************************************/
#else
BOOL FAR recRecognizeData(HWND hWnd, HPENDATA hpendata,
                          LPGUIDE lpg, BOOL UseUpperLevel, BOOL TabletCoord)
{
	RC            rcx;
	short         OldDebugLevel;

	staSetStatus(ST_RECOGNIZING, (LPARAM) 1);
	staSetStatus(ST_FIRSTWORDFROMREC, (LPARAM) 1);
	staSetStatus(ST_MAIL, (LPARAM) 0);
	tlsGetLevels();               /* ??AS */
	if (UseUpperLevel)
	{
		gUseUpper = TRUE;
		OldDebugLevel = DebugLevel; // run Upper regardless of DebugLevel
		DebugLevel = -10;           //??????SD -10 ??
	}
	tlsUpdateStatusBar(BUSY);
	msgStartDebug();
	InitRC(hWnd, &rcx);
	rcx.hrec = hrecParaPen;
	rcx.wResultMode = RRM_WORD;
	if (TabletCoord)
	{
		rcx.lRcOptions |= RCO_TABLETCOORD;
	}
	if (lpg != NULL)
	{
		rcx.lRcOptions |= RCO_BOXED;
		_fmemcpy(&(rcx.guide), lpg, sizeof(GUIDE));
	}
	RecognizeData(&rcx, hpendata);
	glpGuides = lpg;
	if (UseUpperLevel)
	{
		DebugLevel = OldDebugLevel;
		gUseUpper = FALSE;
		msgUpdateLevels();
	}
	tlsUpdateStatusBar(IDLE);
	gWordCut = FALSE; /* return to default */

	staSetStatus(ST_RECOGNIZING, (LPARAM) 0);
	staSetStatus(ST_MAIL, (LPARAM) 1);
	return TRUE;
} /* recRecognizeData */
#endif

/*******************************************************************/
HANDLE FAR recInit(LPSTR lpszRecname)
{
#ifdef _PENWIN
	HREC hrec;

	if (!lpszRecname)
	{
		return NULL;
	}
	if (!*lpszRecname)
	{
		return NULL;
	}
	if (access(lpszRecname, 0) != 0)
	{
		return NULL;
	}
	hrec = InstallRecognizer((LPSTR) lpszRecname);
	if ((int) hrec < 32)
	{
		hrec = NULL;
	}
	else
	{
		WritePrivateProfileString("Recognizer List", lpszRecname, " ", "penwin.ini");
		RegisterPenApp(RPA_DEFAULT, TRUE);
	}
	return hrec;
#endif
	return NULL;
} /* recInit */

/*******************************************************************/
#ifdef _PENWIN
void FAR recDone(HREC hrec)
{
	if (hrec != NULL)
	{
		RegisterPenApp(RPA_DEFAULT, FALSE);
		UninstallRecognizer(hrec);
		SetDebugMode = NULL;
		if (*zRecognizerFakeName && bUseFake)
		{
			WritePrivateProfileString("Recognizer List", (LPSTR) zRecognizerFakeName, \
			                          NULL, "penwin.ini");
			_unlink((LPSTR) zRecognizerFakeName);
		}
	}
} /* recDone */
#endif

/*******************************************************************/
#ifdef _PENWIN
BOOL FAR recLoadRecognizer(LPSTR recfilename)
{
	int ErrMode;
	HREC hrec;
	char buffer[_MAX_PATH];
	char fake[_MAX_PATH];
	BOOL result;
	LPSTR pini, pathname;
	BOOL  LoadIni = TRUE;
	BOOL  FirstAttempt = TRUE;
	HCURSOR hLocalCur;
	/* connect to the recognizer library */

	ErrMode = SetErrorMode(SEM_NOOPENFILEERRORBOX);
	hrec = NULL;
	result = TRUE;
	while ((result == TRUE) && !hrec)
	{

		if (recfilename)
		{
			if (*recfilename)
			{
				hLocalCur = SetCursor(LoadCursor(NULL, IDC_WAIT));
				pini = IniLoad("Recognizer List", NULL, "penwin.ini");
				if (pini)
				{
					pathname = recFindInIni(pini, recfilename, (LPSTR) buffer);
					DebugFreePtr(pini, "recLoadRecognizer");
				}

				if (pathname)
				{
					recfilename = pathname;
				}
				*fake = 0;
				if (prfCreateFakeDLL())
				{
					if (CreateFakeDll(recfilename, (LPSTR) fake))
					{
						if (*fake)
						{
							bUseFake = TRUE;
							hrec = recInit(fake);
						}
					}
				}
				else
				{
					lstrcpy((LPSTR) fake, recfilename);
					if (*fake)
					{
						bUseFake = FALSE;
						hrec = recInit(fake);
					}
				}
				SetCursor(hLocalCur);
			}
		}
		else
		{
			recfilename = (LPSTR) buffer;
			*recfilename = 0;
		}
		if (hrec)
		{
			/* recognizer already loaded */
			if (hrecParaPen)
			{
				recDone(hrecParaPen);
			}
			hrecParaPen = hrec;

			SetDebugMode = (dbgSetDebugMode_TYPE) Q_GetProcAddress(hrec, SETDEBUGMODE_NAME);

			if (SetDebugMode != NULL)
			{
				/* unload prev recognizer if one */
				lstrcpy((LPSTR) zRecognizerFakeName, (LPSTR) fake);
				recSetRecPathName((LPSTR) recfilename);
				/* perform recognizer dll initialization */
				LookLevel = 0;
				tlsSetLevels();
				msgStartDebug();
				msgUpdateLevels();
			}
			else
			{
				/* this is not penlab's recognizer */
				lstrcpy((LPSTR) zRecognizerFakeName, (LPSTR) fake);
				recSetRecPathName((LPSTR) recfilename);
			}
			if (!hLastDebugWnd)
			{
				msgCommand(hMainWnd, WM_COMMAND, IDM_OPEN_INKOUTPUT, 0L);
			}
			dboSetCaption(hLastDebugWnd, NULL);
			SetErrorMode(ErrMode);
			if (!LoadIni && lstrlen(zPsIniName))
				// restore old PS.INI file
			{
				recChangeIniFile(NULL, NULL, zPsIniName, FALSE);
			}
			return TRUE;
		}
		else
		{
			/* test ini settings for default rec loading */
			if (lstrlen(zRecognizerPathName))
			{
				if (!FirstAttempt)
				{
					goto dialog;
				}
				FirstAttempt = FALSE;
				pini = IniLoad("Recognizer List", NULL, "penwin.ini");
				if (pini)
				{
					pathname = recFindInIni(pini, (LPSTR) zRecognizerPathName,
					                        (LPSTR) buffer);
					DebugFreePtr(pini, "recLoadRecognizer");
					if (pathname)
					{
						lstrcpy(recfilename, zRecognizerPathName);
					}
					else
					{
						*zRecognizerPathName = 0;
					}
				}
				else
				{
					*zRecognizerPathName = 0;
				}
			}
			else
			{
dialog:
				result = ircCreateIrcDialog(hMainWnd, recfilename, &LoadIni);
				if (result && LoadIni)
				{
					recSetIniFileName(zPsIniName, recfilename);
				}
			}
		}
	}
	SetErrorMode(ErrMode);
	return FALSE;
	// ???SD what to do?  return (BOOL)(hrecParaPen != NULL);
}
#endif
/* *************************************************************** */
int FAR recGetRecPathName(LPSTR buffer)
{
	if (buffer)
	{
		lstrcpy(buffer, (LPSTR) zRecognizerPathName);
	}
	return lstrlen(zRecognizerPathName);
}

/* *************************************************************** */
int FAR recSetRecPathName(LPSTR buffer)
{
	if (buffer)
	{
		lstrcpy((LPSTR) zRecognizerPathName, buffer);
	}
	return lstrlen(zRecognizerPathName);
}

/* *************************************************************** */
#define MAX_86SEG 0xfff0L
#define MAX_INIENTRY 0x100L

/* *************************************************************** */
LPSTR IniLoad(LPSTR section, LPSTR entry, LPSTR inifile)
{
	LPSTR buffer;
	DWORD mem;
	if (!section)
	{
		return NULL;
	}
	if (!*section)
	{
		return NULL;
	}
	if (!inifile)
	{
		return NULL;
	}
	if (!*inifile)
	{
		return NULL;
	}
	mem = MAX_86SEG;
	if (entry)
	{
		if (!*entry)
		{
			mem = MAX_INIENTRY;
		}
		else
		{
			entry = NULL;
		}
	}
	buffer = (LPSTR) DebugAllocPtr(GPTR, mem, "WGREC IniLoad");
	if (buffer)
	{
		*buffer = 0;
		GetPrivateProfileString(section, entry, "", buffer, (int) mem, inifile);
	}
	return buffer;
}

/* *************************************************************** */
LPSTR recFindInIni(LPSTR pini, LPCSTR filename, LPSTR fullname)
{
	LPSTR   ptr;
	char    buffer[_MAX_PATH];
	char    buffera[_MAX_PATH];

	if (!pini)
	{
		return NULL;
	}
	if (!filename)
	{
		return NULL;
	}
	lstrcpy(buffera, filename);
	_strupr(buffera);
	ptr = pini;
	while (*ptr)
	{
		lstrcpy(buffer, ptr);
		_strupr(buffer);
		if (!lstrcmp(buffer, buffera))
		{
			if (fullname)
			{
				lstrcpy(fullname, buffer);
				return (LPSTR) fullname;
			}
			else
			{
				return NULL;
			}
		}
		else
			if (_fstrstr(buffer, buffera) != NULL)
			{
				if (fullname)
				{
					lstrcpy(fullname, buffer);
					return (LPSTR) fullname;
				}
				else
				{
					return NULL;
				}
			}
		ptr += lstrlen(ptr);
		ptr++;
	}
	return NULL;
} /* end of recFindInIni */

/*********************************************************************/
#ifdef _PENWIN
FARPROC recPenLabRec(HREC hrec)
{
	if (hrec)
	{
		return GetProcAddress(hrec, SETDEBUGMODE_NAME);
	}
	else
	{
		return NULL;
	}
}
#endif
/* *************************************************************** */
#ifdef _PENWIN
void FAR recConfig(void)
{
	typedef UINT(WINAPI *tConfigRecognizer)(UINT, UINT, LONG);
	tConfigRecognizer fnConfigRecognizer = NULL;

	if (hrecParaPen != NULL)
	{
		fnConfigRecognizer = (tConfigRecognizer) GetProcAddress(hrecParaPen,
		                     "ConfigRecognizer");
		if (fnConfigRecognizer)
		{
			fnConfigRecognizer(WCR_PRIVATE + PS_SERV_DB, 0, 0l);
		}
	}
} /* recConfig */
#endif

#include <direct.h>
#include <io.h>
/* ***************************************************************** */
//      recChangeIniFile(hMainWnd, (LPSTR)filename, NULL, TRUE);
/**/
BOOL FAR recChangeIniFile(HWND hWnd, LPSTR lpszIniName, LPSTR lpActualIniName, BOOL ReloadDTE)
{
	LPSTR    lpsz;
	char     szIniName[_MAX_PATH];
	char     OldPsIniName[_MAX_PATH];
	DWORD    size;
	BOOL     result;

#ifndef _PENWIN
	CLOSEREC  lpfnClose = NULL;
	INITREC   lpfnInit = NULL;
	if (hrecParaPen)
	{
		lpfnClose = (CLOSEREC) Q_GetProcAddress(hrecParaPen, REC_CLOSE);
		lpfnInit = (INITREC) Q_GetProcAddress(hrecParaPen, REC_INIT);
	}

#else
	typedef UINT(WINAPI *tConfigRecognizer)(UINT, UINT, LONG);
	tConfigRecognizer fnConfigRecognizer = NULL;
	if (hrecParaPen)
	{
		fnConfigRecognizer = (tConfigRecognizer) GetProcAddress(hrecParaPen, "ConfigRecognizer");
	}
#endif

	if (lpActualIniName)
	{
		// request after "Edit PS ini" dialog box
		result = TRUE;
		if (lpszIniName != NULL)
		{
			lstrcpy(zPsIniName, lpszIniName);
		}
		lstrcpy(OldPsIniName, zPsIniName);
		lpsz = lpActualIniName;
#ifndef _PENWIN
		if (lpfnClose)
		{
			(*lpfnClose)(lpRCinfo);
		}
		if (lpfnInit)
		{
			result = (*lpfnInit)(lpsz, lpRCinfo);
		}
		if (result == FALSE)
		{
			lstrcpy(zPsIniName, OldPsIniName);
			(*lpfnInit)(zPsIniName, lpRCinfo);
			pseUpdatePSEditBufer(zPsIniName);
		}
#else
		if (fnConfigRecognizer)
		{
			if (ReloadDTE)
			{
				fnConfigRecognizer(WCR_PRIVATE + PS_RELOAD_DAT, 7777, 1L);
			}
			fnConfigRecognizer(WCR_PRIVATE + PS_PSINI_CHANGE, 7777, (LONG) lpsz);
			if (ReloadDTE)
			{
				fnConfigRecognizer(WCR_PRIVATE + PS_RELOAD_DAT, 7777, 0L);
			}
		}
#endif
		return result;
	}
	if (hWnd == NULL)
	{
		/* use ps.ini from working directory */
		_getcwd(szIniName, _MAX_DIR);
		size = lstrlen(szIniName);
		if (szIniName[size - 1] != '\\')
		{
			szIniName[size] = '\\';
			szIniName[size + 1] = '\0';
		}
		lstrcat(szIniName, "ps.ini");
		/* check file existence */
		if (_access(szIniName, 0) != 0)
		{
			char buff[2 * _MAX_PATH];   //CHE
			wsprintf(buff, "No such file: \"%s\".", szIniName);
			MessageBox(hWnd, buff, "LOAD PS.INI", MB_ICONSTOP | MB_OK);
			return FALSE;
		}
		else
		{
			lpsz = szIniName;
		}
	}
	else
		if (lpszIniName != NULL)
		{
			/* check file existence */
			if (_access(lpszIniName, 0) != 0)
			{
				goto dlgbrowse;
			}
			else
			{
				//ayv reload the ini file every time
				if (lstrcmpi(lpszIniName, zPsIniName) == 0)
				{
					// same PS.INI
					// no need to reload it
					if (prfReloadPsIniWhenModified())
					{
						if (!psePsIniModified())
						{
							return TRUE;
						}
					}
					else
					{
						return TRUE;
					}
				}
				//#endif
				lpsz = lpszIniName;
			}
		}
		else
		{
dlgbrowse:
			if (recGetIniFileName(hWnd, szIniName))
			{
				lpsz = szIniName;
			}
			else
			{
				return  FALSE;
			}
		}
#ifndef _PENWIN
	lstrcpy(OldPsIniName, zPsIniName);
	if (lpfnClose)
	{
		(*lpfnClose)(lpRCinfo);
	}
	if (lpfnInit)
	{
		result = (*lpfnInit)(lpsz, lpRCinfo);
		if (result)
		{
			lstrcpy(zPsIniName, lpsz);
			pseUpdatePSEditBufer(lpsz);
		}
		else
		{
			lstrcpy(zPsIniName, OldPsIniName);
			(*lpfnInit)(zPsIniName, lpRCinfo);
			pseUpdatePSEditBufer(zPsIniName);
			return FALSE;
		}
	}
#else
	if (fnConfigRecognizer)
	{
		fnConfigRecognizer(WCR_PRIVATE + PS_PSINI_CHANGE, 7777, (LONG) lpsz);
		lstrcpy(zPsIniName, lpsz);
		pseUpdatePSEditBufer(lpsz);
	}
#endif
	return TRUE;
} /* end of recChangeIniFile */
/*=================================================================*/
LPGUIDE  FAR recGetGuidesPtr(void)
{
	return glpGuides;
}  /* end of recGetGuidesPtr */

/*******************************************************************/
BOOL FAR recGetIniFileName(HWND hWnd, LPSTR szIniName)
{
	OPENFILENAME    ofn;

	*szIniName = 0;
	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.hInstance = hInst;
	ofn.lpstrFilter = (LPSTR) "INI Files(*.INI)\0*.ini\0";
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 1;
	ofn.nMaxFile = _MAX_PATH;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrFileTitle = NULL;
	ofn.lpstrFile = szIniName;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrTitle = NULL;
	ofn.lpstrDefExt = (LPSTR) "ini";
	ofn.Flags = OFN_READONLY;
	ofn.lpTemplateName = NULL; //(LPSTR) "FILEOPENORD";
	return GetOpenFileName(&ofn);
} /* end of GetIniFileName */

/***********************************************************************/
void FAR recSetPsIniName(LPSTR buffer)
{
	if (_access(buffer, 0) == 0)
	{
		lstrcpy(zPsIniName, buffer);
	}
} /* end of recSetPsIniName */

/***********************************************************************/
void FAR recGetPSIniName(LPSTR IniName)
{
	lstrcpy(IniName, zPsIniName);
} /* end of recGetPSIniName */

/***********************************************************************/
void FAR recSetIniFileName(LPSTR IniName, LPSTR RecName)
{
	LPSTR p;
	lstrcpy(IniName, RecName);
	p = IniName + lstrlen(IniName) - 1;
	while (*p != '\\' && *p != ':')
	{
		if (p == IniName)
		{
			lstrcpy(IniName, PS_INI_NAME);
			return;
		}
		p--;
	}
	*(++p) = 0;
	lstrcat(IniName, PS_INI_NAME);
} /* end of recSetIniFileName */


/***********************************************************************/
/***********************************************************************/
/***********************   NT recognizer  ******************************/
/***********************************************************************/
/***********************************************************************/

void  FAR recClose(void)
{
	CLOSEREC lpfnClose;
	if (hrecParaPen != NULL)
	{
		lpfnClose = (CLOSEREC) Q_GetProcAddress(hrecParaPen, REC_CLOSE);
		(*lpfnClose)(lpRCinfo);
		DISPOSE(lpRCinfo, "WGREC recClose");
		Q_FreeLibrary(hrecParaPen);
		hrecParaPen = NULL;
	}

} /* end of recClose */


/***********************************************************************/
#ifndef _PENWIN
static int FindRecNameInIniFile(LPSTR RecognizerName)
{
	char  Names[MAX_LIST_SIZE];
	char  drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME + 1 + _MAX_EXT], ext[_MAX_EXT];
	LPSTR lpName, lpNext;
	int   result, temp = 0;

	result = ircReadAvailableRecogs(Names);
	RETURN_IF_BAD_RESULT(result, FALSE);
	lpName = lpNext = Names;
	while (*lpNext && temp < MAX_RECS)
	{
		while (*lpNext)
		{
			lpNext++;
		}
		_splitpath(lpName, drive, dir, fname, ext);
		lstrcat(fname, ext);
		if (lstrcmpi(fname, RecognizerName) == 0)
		{
			if (_access(lpName, 0) == 0)
			{
				lstrcpy(RecognizerName, lpName);
				return TRUE;
			}
		}
		temp++;
		lpNext++;
		if (*lpNext)
		{
			lpName = lpNext;
		}
	}
	return FALSE;
}  /* end of FindRecNameInIniFile */
#endif

/***********************************************************************/

BOOL FAR recLoadRecognizerNT(LPSTR RecognizerName)
{
	INITREC lpfnInit;
	BOOL    result, LoadIni = TRUE;
	char    TempRecognizerName[_MAX_PATH], buff[2 * _MAX_PATH];
	char    OldPSIniName[_MAX_PATH];
	LPSTR   p;

	lstrcpy(OldPSIniName, zPsIniName);
	if (RecognizerName == NULL || lstrlen(RecognizerName) == 0)
	{
		result = prfLoadLastRecognizer();
		if (!result || (result && !lstrlen(zRecognizerPathName)))
		{
			// call dialog box with recognizers list
			result = ircCreateIrcDialog(hMainWnd, zRecognizerPathName, &LoadIni);
			if (result)
			{
				if (LoadIni || !lstrlen(zPsIniName))
				{
					// set PS.INI name
#ifdef STATIC_LINKAGE
					lstrcpy(zPsIniName, zRecognizerPathName);
#else
					recSetIniFileName(zPsIniName, zRecognizerPathName);
#endif // STATIC_LINKAGE
				}
			}
			else
			{
				return FALSE;
			}
		}
	}
	else
	{
		// "load recognizer" request from command line in BATCH mode
		// check if RecognizerName is full path name,
		// if not - search for it in PENLAB.INI [Recognizer List] section
		lstrcpy(TempRecognizerName, RecognizerName);
		p = _fstrchr(TempRecognizerName, ':');
		if (p != NULL)
		{
			lstrcpy(zRecognizerPathName, TempRecognizerName);
		}
		else
		{
			result = FindRecNameInIniFile(TempRecognizerName);
			if (result == FALSE)
			{
				wsprintf(buff, "Can't find %s file\n in PenLab32.ini file", TempRecognizerName);
				MessageBox(hMainWnd, buff, "BATCH PROCESSING", MB_ICONHAND);
				return FALSE;
			}
			else
				if (lstrcmpi(zRecognizerPathName, TempRecognizerName) == 0)
				{
					// same recognizer: no reload, init new PS.INI only
					//??SD ???
					if (gPenLabBatchMode)
					{
						// in wgbatch new PS.INI name was set before calling recLoadRec
						// so we need to reload ps.ini
						goto init;
					}
					else
					{
						// batch from tester PS.INI was already reload by call to recChangeINI
						// from msgPARSECMDLINE
						return TRUE;
					}
				}
			lstrcpy(zRecognizerPathName, TempRecognizerName);
		}
	}


	if (hrecParaPen != NULL)
	{
		// remove old recognizer
		recClose();
	}

	hLocalWaitCur = LoadCursor(hInst, IDC_WAIT);
	hLocalCur = SetCursor(hLocalWaitCur);
	// create lpRCinfo
	lpRCinfo = (rec_info_type *)DebugAllocPtr(GMEM_ZEROINIT | GMEM_SHARE | GMEM_FIXED, sizeof(rec_info_type), "WGREC recLoadRecognizerNT");
	if (lpRCinfo == NULL)
	{
		goto error;
	}
	hrecParaPen = Q_LoadLibrary(zRecognizerPathName);
	if (hrecParaPen == NULL)
	{
		wsprintf(buff, "Can't load %s", zRecognizerPathName);
		MessageBox(hMainWnd, buff, "LOAD RECOGNIZER", MB_ICONSTOP);
		goto error;
	}
	//??SD
	SetDebugMode = (dbgSetDebugMode_TYPE) Q_GetProcAddress(hrecParaPen, SETDEBUGMODE_NAME);
	LookLevel = 0;
	tlsSetLevels();
	msgStartDebug();
	if (!hLastDebugWnd)
	{
		msgCommand(hMainWnd, WM_COMMAND, IDM_OPEN_INKOUTPUT, 0L);
	}
	dboSetCaption(hLastDebugWnd, NULL);


init:
	lpfnInit = (INITREC) Q_GetProcAddress(hrecParaPen, REC_INIT);
	if (lpfnInit == NULL)
	{
		Q_FreeLibrary(hrecParaPen);
		hrecParaPen = NULL;
		goto error;
	}

	if (lstrlen(zPsIniName) == 0)
	{
		recSetIniFileName(zPsIniName, zRecognizerPathName);
	}

	result = (*lpfnInit)(zPsIniName, lpRCinfo);

	if (result)
	{
		if (!LoadIni && lstrlen(zPsIniName))
		{
			recChangeIniFile(NULL, NULL, zPsIniName, FALSE);
		}
		SetCursor(hLocalCur);
		return TRUE;
	}
	else
	{
		wsprintf(buff, "Can't init RC with %s", zPsIniName);
		MessageBox(hMainWnd, buff, "INIT RECOGNIZER", MB_ICONSTOP);
		lstrcpy(zPsIniName, OldPSIniName);
		if (lstrlen(zPsIniName))
		{
			(*lpfnInit)(zPsIniName, lpRCinfo);
		}
	}
error:
	SetCursor(hLocalCur);
	return FALSE;
}  /* end of recLoadRecognizerNT */

/***********************************************************************/

BOOL FAR recRecognizeCommData(p_TRACEDATA lpCommTrace)
{
	BOOL              result;
	HWND              hWndDebug;

	if (lpRCinfo == NULL || lpRCinfo->pTrace == NULL)
	{
		return FALSE;
	}
	if (lpCommTrace->TDnp <= MIN_COM_INPUT)
	{
		return FALSE;
	}
	msgHWDebug(hWndClient, WM_HWDBG, 0, 0L);
	hWndDebug = GetDlgItem(hLastDebugWnd, DRAWINK);
	if (lpRCinfo == NULL)
	{
		// no installed recognizer
		result = recLoadRecognizerNT(NULL);
		if (!result)
		{
			return result;
		}
	}
	hlvSetCurSect(-1, 0);
	result = SetRCinfoTrace(lpCommTrace);
	RETURN_IF_BAD_RESULT(result, 0);
	result = recRecognizeDataNT(hWndDebug, FALSE, TRUE);
	return result;
} /* recRecognizeCommData */

/***********************************************************************/

BOOL  FAR recGetCapsAndAmnisty(LPINT pData)
{
	RCBACCESS       lpfnRC;
	int             result;

	RETURN_IF_BAD_POINTER(hrecParaPen);
	RETURN_IF_BAD_POINTER(lpRCinfo);
	lpfnRC = (RCBACCESS) Q_GetProcAddress(hrecParaPen, REC_DATA_ACCESS);
	RETURN_IF_BAD_POINTER(lpfnRC);

	// set recognizer trace
	result = (*lpfnRC)((void FAR *)lpRCinfo, OBJ_RCONTEXT, CMD_INF,
	                   (UINT) 0, (ULONG) pData);
	return (result == 0);
} /* end of recGetCapsAndAmnisty() */

/*******************************************************************/
/**        Function substitution for static linkage.			  **/
/*******************************************************************/

HINSTANCE Q_LoadLibrary(LPCSTR libname)
{
#ifdef STATIC_LINKAGE
	return (HINSTANCE) 1;
#else
	return LoadLibrary(libname);
#endif
}

BOOL Q_FreeLibrary(HINSTANCE hdll)
{
#ifdef STATIC_LINKAGE
	return TRUE;
#else
	return FreeLibrary(hdll);
#endif
}

FARPROC Q_GetProcAddress(HMODULE hmod, LPCSTR fname)
{
#ifdef STATIC_LINKAGE

	if (strcmp(fname, SETDEBUGMODE_NAME) == _NULL)
	{
		return (FARPROC) dbgSetDebugMode;
	}

	if (strcmp(fname, REC_INIT) == _NULL)
	{
		return (FARPROC) InitRecognition;
	}

	if (strcmp(fname, REC_CLOSE) == _NULL)
	{
		return (FARPROC) CloseRecognition;
	}

	if (strcmp(fname, REC_RECOGNIZE) == _NULL)
	{
		return (FARPROC) DoRecognition;
	}

	if (strcmp(fname, REC_DATA_ACCESS) == _NULL)
	{
		return (FARPROC) RCBAccess;
	}

	return (FARPROC) _NULL;
#else
	return GetProcAddress(hmod, fname);
#endif
}


/***********************************************************************/

