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

#define  STRICT
#define  _REQ_WIN
#include <windows.h>
#include <windowsx.h>
#ifndef _PENWIN
#include "pensub.h32"
#else
#include <penwin.h>
#include <penwoem.h>
#include <xrword.h>
#include <learn.h>
#endif
#include <hwr_sys.h>
#include <ams_mg.h>

#include "topine.h"
#include <vocutilp.h>
#include <direct.h>
#include <stdio.h>
#include <io.h>
#include <string.h>
#include <stdlib.h>
#include <commdlg.h>
#include <limits.h>
#include <bastypes.h>
#include <wg_stuff.h>
#include "wggbl.h"
#include "wgmsg.h"
#include "wgtrc.h"
#include "wgrec.h"

#include "zxlat.h"
#include "wgvoc.h"


BOOL CALLBACK  VocDlgProc(HWND, UINT, WPARAM, LPARAM);

#define      MODE_BIN   1
#define      MODE_TXT   2

#define      SZ_WORD 128
#define      VOC_FREE(x)  if(x){D_Dfree(x);x=(_HD)0;}

static char  szM0 [] = "BIN to Text";
static char  szM1 [] = "Text to BIN";

static void  Buttons(void);
static LPSTR VocGetWord(void);
static int   VocGetByte(void);
static int   VocGetParm(void);
static void  VocAddWord(LPSTR, int);
static void  VocTstWord(LPSTR, int);
static void  VocDelWord(LPSTR, int);
static _HD   GetVocL(void);
static _HD   GetVocR(void);
static LPSTR GetVocName(void);
static LPSTR Reverse(LPSTR szWord);
static void  VocLoad(void);
static void  VocSave(void);
static BOOL  VocLoadBin(_HD *phVocL, _HD *phVocR);
static void  VocSaveBin(_HD, _HD);
static BOOL  VocLoadTxt(_HD *phVocL, _HD *phVocR);
static void  VocSaveTxt(_HD, _HD);
static void  VocTran(void);
static LPSTR DelPoint(LPSTR lpName);
static void  GetMode(int*, int*);
static LPSTR SetName(LPSTR szName);

static HWND  hWnd;

static LPCSTR lpWord;

LPSTR     VocNameForLoad(HWND hWnd);
LPSTR     VocNameForSave(HWND hWnd);
LPSTR     LstNameForLoad(HWND hWnd);
LPSTR     LstNameForSave(HWND hWnd);

/*****************************************************************************/
/*** this part depend of recognizer interface ********************************/
/*****************************************************************************/
extern dbgSetDebugMode_TYPE    SetDebugMode;

vocptr_type _PTR GetVocPtr(void)
{
	static vocptr_type _PTR vc;

#ifdef _PENWIN
	static rc_type          rc;

	if (SetDebugMode != NULL)
	{
		SetDebugMode(HWDBG_GETRC,(LPARAM)(p_VOID)&rc);
	}
	else
	{
		return NULL;
	}
	vc = rc.vocptr[0];
#else
	p_rec_info_type    pRCinfo;
	RCBACCESS          lpfnRC;
	HINSTANCE          hDLL;

	pRCinfo = recGetRCinfo();
	hDLL = recGetDLLHandle();
	lpfnRC = (RCBACCESS) Q_GetProcAddress(hDLL, REC_DATA_ACCESS);
	(*lpfnRC)((void FAR *)pRCinfo, OBJ_RCONTEXT, CMD_VOC, 0, (long) &vc);
#endif

	return vc;
}

void SetVocPtr(vocptr_type _PTR vc)
{
#ifdef _PENWIN
	static rc_type          rc;

	if (SetDebugMode != NULL)
	{
		SetDebugMode(HWDBG_GETRC,(LPARAM)(p_VOID)&rc);
	}
	else
	{
		return NULL;
	}
	rc.vocptr[0] = vc;
#else
	p_rec_info_type    pRCinfo;
	RCBACCESS          lpfnRC;
	HINSTANCE          hDLL;

	pRCinfo = recGetRCinfo();
	hDLL = recGetDLLHandle();
	lpfnRC = (RCBACCESS) Q_GetProcAddress(hDLL, REC_DATA_ACCESS);
	(*lpfnRC)((void FAR *)pRCinfo, OBJ_RCONTEXT, CMD_VOC, 1, (long) &vc);
#endif
}

void VocLoad(void)
{
	static vocptr_type _PTR vc;
	static VOCLOAD_TYPE voc;

	LPSTR          lpFile;

	SetName("");
	lpFile = VocNameForLoad(hWnd);

	if (!lpFile)
	{
		return;
	}
	if ((vc = GetVocPtr()) == _NULL)
	{
		return;
	}

	voc.vocl_ptr = 0;
	voc.vocl_filename = lpFile;

	UpdateWindow(hWnd);
	if (SetDebugMode == NULL)
	{
	}
	else
	{
		SetDebugMode(HWDBG_FREEVOC, (LPARAM) (p_VOID) &vc);
		SetDebugMode(HWDBG_LOADVOC, (LPARAM) (p_VOCLOAD) &voc);
		SetVocPtr((vocptr_type*) voc.vocl_ptr);
	}
	Buttons();
}
/*****************************************************************************/
int VocService(HWND hParentWnd, LPCSTR szWord)
{
	int       RetCode;
	DLGPROC   lpVocDlgProc;

	lpWord = szWord;
	lpVocDlgProc = (DLGPROC) MakeProcInstance((FARPROC) VocDlgProc, hInst);
	if ((RetCode = DialogBox(hInst, (LPCSTR)"WGVOC_DLG", hParentWnd, lpVocDlgProc)) == -1)
	{
		MessageBox(NULL, "Unable to display dialog", "System Error",
		           MB_SYSTEMMODAL | MB_ICONHAND | MB_OK);
		return FALSE;
	}

	FreeProcInstance((FARPROC) lpVocDlgProc);
	return(RetCode);
}


BOOL CALLBACK VocDlgProc(HWND hDlg, UINT wMessage, WPARAM wParam, LPARAM lParam)
{
	switch (wMessage)
	{
		case WM_INITDIALOG:
		{
			RECT rc;
			int  x, y, dx, dy;

			hWnd = hDlg;
			hWnd = GetDlgItem(hDlg, IDC_VOC_WORD);
			//                if(lpWord)
			//                   SetWindowText(hWnd, lpWord);

			hWnd = hDlg;
			GetWindowRect(hDlg, (LPRECT) &rc);
			dx = rc.right - rc.left;
			dy = rc.bottom - rc.top;
			x = (GetSystemMetrics(SM_CXSCREEN) - dx) / 2;
			y = (GetSystemMetrics(SM_CYSCREEN) - dy) / 2;
			MoveWindow(hDlg, x, y, dx, dy, FALSE);

			SendMessage(GetDlgItem(hDlg, IDC_VOC_MODE), CB_ADDSTRING, 0, (LPARAM) (LPCSTR) szM0);
			SendMessage(GetDlgItem(hDlg, IDC_VOC_MODE), CB_ADDSTRING, 0, (LPARAM) (LPCSTR) szM1);
			SendMessage(GetDlgItem(hDlg, IDC_VOC_MODE), CB_SETCURSEL, 0, 0L);
			Buttons();
		}
		return TRUE;

		case WM_COMMAND:
			switch (wParam)
			{
				case IDOK:
					EndDialog(hDlg, TRUE);
					return TRUE;

				case IDCANCEL:
					EndDialog(hDlg, FALSE);
					return TRUE;

				case IDC_VOC_ADD:
					VocAddWord(VocGetWord(), VocGetParm());
					return TRUE;
				case IDC_VOC_TST:
					VocTstWord(VocGetWord(), VocGetParm());
					return TRUE;
				case IDC_VOC_DEL:
					VocDelWord(VocGetWord(), VocGetParm());
					return TRUE;

				case IDC_VOC_WORD:
					Buttons();
					return TRUE;

				case IDC_VOC_LOAD:
					VocLoad();
					return TRUE;
				case IDC_VOC_SAVE:
					VocSave();
					return TRUE;
				case IDC_VOC_TRAN:
					VocTran();
					return TRUE;

				default:
					return FALSE;

			}

		case WM_SYSCOMMAND:

			switch (wParam & 0xFFF0)
			{
				case SC_CLOSE:
					EndDialog(hDlg, FALSE);
					return TRUE;
			}
	}
	return FALSE;
}

// Implementations:

void Buttons()
{
	BOOL                bFlg;

	bFlg = TRUE;//( lstrcmp(VocGetWord(),"") != 0 );

	EnableWindow(GetDlgItem(hWnd, IDC_VOC_ADD), bFlg);
	EnableWindow(GetDlgItem(hWnd, IDC_VOC_TST), bFlg);
	EnableWindow(GetDlgItem(hWnd, IDC_VOC_DEL), bFlg);
	SetWindowText(GetDlgItem(hWnd, IDC_VOC_NAME), GetVocName());
}

LPSTR VocGetWord()
{
	static char szBuff[SZ_WORD];
	static int  i;

	GetDlgItemText(hWnd, IDC_VOC_WORD, (LPSTR) szBuff, SZ_WORD - 1);
	for (i = 0; szBuff[i]; i++)
	{
		szBuff[i] = (char) ANSIToMAC(szBuff[i]);
	}

	return(szBuff);
}

int VocGetByte()
{
	return((int) GetDlgItemInt(hWnd, IDC_VOC_BYTE, NULL, FALSE));
}

int   VocGetParm()
{
	return(VocGetByte());
}

void VocAddWord(LPSTR szWord, int nByte)
{
	char   szBuff[128];
	_HD    hVocL = GetVocL();
	//    _HD    hVocR = GetVocR();
	_SHORT nParm;
	int    bRet = IDYES;

	if (!hVocL)
	{
		return;
	}
	//    if(!hVocR) return;

	if (D_GetNextTail((p_UCHAR) szWord, (p_SHORT) &nParm, hVocL))
	{
		wsprintf((LPSTR) szBuff, (LPSTR)"Word is present: %s [Weight: %d], owerwrite?", (LPSTR) szWord, nParm);
		bRet = MessageBox(hWnd, (LPSTR) szBuff, (LPSTR)"Vocabulary Service", MB_YESNO | MB_ICONINFORMATION);
	}

	if (bRet == IDNO)
	{
		return;
	}

	if (D_AddWord((p_UCHAR) szWord, (short) nByte, hVocL) == 0)
	{
		MessageBox(hWnd, (LPSTR)"ELK: word add error... \nGot optimized dictionary?", (LPSTR) szWord, MB_OK | MB_ICONSTOP);
	}
	else
	{
		MessageBox(hWnd, (LPSTR)"ELK: Word Added.", (LPSTR) szWord, MB_OK | MB_ICONSTOP);
	}
}

void VocTstWord(LPSTR szWord, int nParm)
{
	char   szBuff[128];
	_HD    hVocL = GetVocL();
	_HD    hVocR = GetVocR();

	if (!hVocL)
	{
		return;
	}
	//    if(!hVocR) return;


	if (D_GetNextTail((p_UCHAR) szWord, (p_SHORT) &nParm, hVocL))
	{
		wsprintf((LPSTR) szBuff, (LPSTR)"Word is present: %s [Weight: %d]", (LPSTR) szWord, nParm);
		MessageBox(hWnd, (LPSTR) szBuff, (LPSTR)"Vocabulary Service", MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		wsprintf((LPSTR) szBuff, (LPSTR)"No word in the dictionary: %s!", (LPSTR) szWord);
		MessageBox(hWnd, (LPSTR) szBuff, (LPSTR)"Vocabulary Service", MB_OK | MB_ICONINFORMATION);
	}
}

void VocDelWord(LPSTR szWord, int nParm)
{
	_HD    hVocL = GetVocL();
	_HD    hVocR = GetVocR();

	MessageBox(hWnd, (LPSTR)"Don't know how to remove words!", (LPSTR)"Delete", MB_OK);

	if (!hVocL)
	{
		return;
	}
	if (!hVocR)
	{
		return;
	}

	if (!D_DelWord((p_UCHAR) szWord, hVocL))
	{
		MessageBox(hWnd, (LPSTR)"No word in the vocabulary!", (LPSTR)"Delete", MB_OK);
	}

	Reverse(szWord);
	if (!D_DelWord((p_UCHAR) szWord, hVocR))
	{
	}
}

_HD GetVocL()
{
#ifdef _PENWIN
	static rc_type          rc;

#endif // _PENWIN
	static vocptr_type _PTR vc;

#ifdef _PENWIN
	if (SetDebugMode != NULL)
	{
		SetDebugMode(HWDBG_GETRC,(LPARAM)(p_VOID)&rc);
	}
	else
	{
		return NULL;
	}
	vc = rc.vocptr[0];
	if (!vc)
	{
		return NULL;
	}
#else // !_PENWIN
	if ((vc = GetVocPtr()) == _NULL)
	{
		return NULL;
	}
#endif // _PENWIN

	//    return((_HD)vc->hvoc_dir);
	return((_HD) vc);
}

_HD GetVocR()
{
#ifdef _PENWIN
	static rc_type          rc;
#endif // _PENWIN
	static vocptr_type _PTR vc;

#ifdef _PENWIN
	if (SetDebugMode != NULL)
	{
		SetDebugMode(HWDBG_GETRC,(LPARAM)(p_VOID)&rc);
	}
	else
	{
		return NULL;
	}
	vc = rc.vocptr[0];
	if (!vc)
	{
		return NULL;
	}
#else // !_PENWIN
	if ((vc = GetVocPtr()) == _NULL)
	{
		return NULL;
	}
#endif // _PENWIN

	//    return((_HD)vc->hvoc_inv);
	return(0);
}

LPSTR GetVocName(void)
{
#ifdef _PENWIN
	static rc_type          rc;
#endif // _PENWIN
	static vocptr_type _PTR vc;

#ifdef _PENWIN
	if (SetDebugMode != NULL)
	{
		SetDebugMode(HWDBG_GETRC,(LPARAM)(p_VOID)&rc);
	}
	else
	{
		return NULL;
	}
	vc = rc.vocptr[0];
	if (!vc)
	{
		return NULL;
	}
#else // !_PENWIN
	if ((vc = GetVocPtr()) == _NULL)
	{
		return NULL;
	}
#endif // _PENWIN
	return((LPSTR) vc->vocname);
}




LPSTR Reverse(LPSTR str)
{
	static char zTemp[SZ_WORD + 1];
	int  i, j;

	zTemp[lstrlen(str)] = 0;

	for (i = lstrlen(str) - 1, j = 0; i >= 0; i--, j++)
	{
		zTemp[j] = str[i];
	}
	lstrcpy(str, zTemp);
	return(str);
}



void VocSave()
{
	SetName("");
	VocSaveBin(GetVocL(), GetVocR());
}


/* names for load & save ----------------------------------------
 */

#define MAXFILENAME 200

static  OPENFILENAME    ofn;

static  LPSTR   lsVocFilter = "DIR (*.L_R)\0*.L_R\0INV (*.R_L)\0*.R_L\0";
static  LPSTR   lsVocExt = NULL;
static  LPSTR   lsVocLoad = "Load Vocabulary (Binary mode)";
static  LPSTR   lsVocSave = "Save Vocabulary (Binary mode)";

static  char    szFileName[MAXFILENAME];
static  char    szFileTitl[MAXFILENAME];

LPSTR   SetName(LPSTR szName)
{
	lstrcpy(szFileName, szName);
	return(szFileName);
}

LPSTR     VocNameForLoad(HWND hWnd)
/*---------------------------------*/
{
	_fmemset((LPOPENFILENAME) &ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = lsVocFilter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = (LPSTR) szFileName;
	ofn.nMaxFile = sizeof(szFileName);
	ofn.lpstrFileTitle = (LPSTR) szFileTitl;
	ofn.nMaxFileTitle = sizeof(szFileTitl);
	ofn.lpstrTitle = lsVocLoad;
	ofn.lpstrDefExt = lsVocExt;
	ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (!GetOpenFileName((LPOPENFILENAME) &ofn))
	{
		return(NULL);
	}
	return((LPSTR) szFileName);
}

LPSTR     VocNameForSave(HWND hWnd)
/*-------------------------------------------------*/
{
	_fmemset((LPOPENFILENAME) &ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = lsVocFilter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = (LPSTR) szFileName;
	ofn.nMaxFile =
	    sizeof(szFileName);
	ofn.lpstrFileTitle = (LPSTR) szFileTitl;
	ofn.nMaxFileTitle = sizeof(szFileTitl);
	ofn.lpstrTitle = lsVocSave;
	ofn.lpstrDefExt = lsVocExt;
	ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

	if (!GetSaveFileName((LPOPENFILENAME) &ofn))
	{
		return(NULL);
	}
	return((LPSTR) szFileName);
}

static  LPSTR   lsLstFilter = "Listing   (*.lst)\0*.LST\0All files (*.*  )\0*.*\0";
static  LPSTR   lsLstExt = NULL;
static  LPSTR   lsLstLoad = "Load Vocabulary (Text mode)";
static  LPSTR   lsLstSave = "Save Vocabulary (Text mode)";

LPSTR     LstNameForLoad(HWND hWnd)
/*---------------------------------*/
{
	_fmemset((LPOPENFILENAME) &ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = lsLstFilter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = (LPSTR) szFileName;
	ofn.nMaxFile = sizeof(szFileName);
	ofn.lpstrFileTitle = (LPSTR) szFileTitl;
	ofn.nMaxFileTitle = sizeof(szFileTitl);
	ofn.lpstrTitle = lsLstLoad;
	ofn.lpstrDefExt = lsLstExt;
	ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;;

	if (!GetOpenFileName((LPOPENFILENAME) &ofn))
	{
		return(NULL);
	}
	return((LPSTR) szFileName);
}

LPSTR     LstNameForSave(HWND hWnd)
/*-------------------------------------------------*/
{
	_fmemset((LPOPENFILENAME) &ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = lsLstFilter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = (LPSTR) szFileName;
	ofn.nMaxFile = sizeof(szFileName);
	ofn.lpstrFileTitle = (LPSTR) szFileTitl;
	ofn.nMaxFileTitle = sizeof(szFileTitl);
	ofn.lpstrTitle = lsLstSave;
	ofn.lpstrDefExt = lsLstExt;
	ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

	if (!GetSaveFileName((LPOPENFILENAME) &ofn))
	{
		return(NULL);
	}
	return((LPSTR) szFileName);
}

LPSTR DelPoint(LPSTR lpName)
{
	int i;

	for (i = lstrlen(lpName) - 1; i >= 0; i--)
	{
		if (lpName[i] == '\\')
		{
			break;
		}
		if (lpName[i] == '/')
		{
			break;
		}
		if (lpName[i] != '.')
		{
			continue;
		}

		lpName[i] = 0;
		break;
	}
	return(lpName);
}


// Translation **************************************************


static _HD  hVocL;
static _HD  hVocR;
static BOOL bLine;

static  int   GetC(FILE *f);
static  int   LoadNum(FILE *f, LPSTR zBuff, int n);
static  int   LoadStr(FILE *f, LPSTR zBuff, int n);
static  LPSTR DirGetExtn(LPSTR zBuff, LPSTR zDir);

void LoopBeg(void);
void LoopCnt(LPSTR lpWord);
void LoopEnd(void);


void VocTran()
{
	int  ModeL, ModeS;
	BOOL bFlg;

	hVocL = hVocR = 0;

	GetMode(&ModeL, &ModeS);

	if (ModeL == MODE_BIN)
	{
		bFlg = VocLoadBin(&hVocL, &hVocR);
	}
	if (ModeL == MODE_TXT)
	{
		bFlg = VocLoadTxt(&hVocL, &hVocR);
	}

	DelPoint(szFileName);
	if (ModeS == MODE_BIN)
	{
		lstrcat(szFileName, ".l_r");
	}
	if (ModeS == MODE_TXT)
	{
		lstrcat(szFileName, ".lst");
	}

	if (ModeS == MODE_BIN && bFlg)
	{
		VocSaveBin(hVocL, hVocR);
	}
	if (ModeS == MODE_TXT && bFlg)
	{
		VocSaveTxt(hVocL, hVocR);
	}

	VOC_FREE(hVocL);
	VOC_FREE(hVocR);

	LoopEnd();
}

void GetMode(int *ModeL, int *ModeS)
{
	int iPos = (int) SendMessage(GetDlgItem(hWnd, IDC_VOC_MODE), CB_GETCURSEL, 0, 0L);

	if (iPos == 1)
	{
		*ModeL = MODE_TXT;
		*ModeS = MODE_BIN;
	}
	else
	{
		*ModeL = MODE_BIN;
		*ModeS = MODE_TXT;
	}
}


BOOL VocLoadTxt(_HD *phVocL, _HD *phVocR)
{
	FILE   *f;
	char    zBuff[16];
	char    zWord[64];
	_SHORT  nWeight;
	BOOL    bError;
	int     i;
	LPSTR   lpFile;
	_HD     hVocL, hVocR;

	*phVocL = *phVocR = NULL;

	SetName("");
	lpFile = LstNameForLoad(hWnd);
	if (!lpFile)
	{
		return(FALSE);
	}

	hVocL = D_Dopen(_NULL);
	hVocR = D_Dopen(_NULL);

	if (!hVocL || !hVocR)
	{
		MessageBox(hWnd, "Can't open dictionary.", NULL, MB_OK);
		*phVocL = hVocL;
		*phVocR = hVocR;
		return(FALSE);
	}
	if ((f = fopen(lpFile, "rt")) == NULL)
	{
		MessageBox(hWnd, "Can't open file for read dictionary...", NULL, MB_OK | MB_ICONSTOP);
		*phVocL = hVocL;
		*phVocR = hVocR;
		return(FALSE);
	}
	LoopBeg();
	for (i = 0;; i++)
	{
		int nRetStr, nRetNum;

		nRetStr = LoadStr(f, zWord, 64);
		nRetNum = LoadNum(f, zBuff, 16);

		if (i)
		{
			bError = nRetStr < 0 || nRetNum < 0 || nRetStr>0 && nRetNum == 0;
		}
		else
		{
			bError = nRetStr <= 0 || nRetNum <= 0;
		}
		if (nRetStr <= 0 || nRetNum <= 0)
		{
			break;
		}

		nWeight = atoi(zBuff);
		// Add:
		LoopCnt(zWord);
		if (D_AddWord((p_UCHAR) zWord, nWeight, hVocL) == 0)
		{
			MessageBox(hWnd, (LPSTR)"PINE: l-word adding error...", (LPSTR) zWord, MB_OK | MB_ICONSTOP);
		}

		Reverse(zWord);

		if (D_AddWord((p_UCHAR) zWord, nWeight, hVocR) == 0)
		{
			MessageBox(hWnd, (LPSTR)"PINE: r-word adding error...", (LPSTR) zWord, MB_OK | MB_ICONSTOP);
		}
		//
	}
	fclose(f);

	if (bError)
	{
		MessageBox(hWnd, "Dictionary file format error...", NULL, MB_OK | MB_ICONSTOP);
		*phVocL = hVocL;
		*phVocR = hVocR;
		return(FALSE);
	}
	*phVocL = hVocL;
	*phVocR = hVocR;
	return(TRUE);
}

void VocSaveTxt(_HD hVocL, _HD hVocR)
{
	FILE   *f;
	_SHORT  nWeight;
	_HD     hVocX;
	_HDB    hDB;
	LPSTR   lpFile = LstNameForSave(hWnd);
	char    szBuff[SZ_WORD];

	if (!lpFile)
	{
		return;
	}

	if (!hVocL)
	{
		hVocX = hVocR;
	}
	else
	{
		hVocX = hVocL;
	}

	if ((f = fopen(lpFile, "wt")) == NULL)
	{
		MessageBox(hWnd, "Can't open file for writing...", NULL, MB_OK | MB_ICONSTOP);
		return;
	}
	D_GetHDB(&hDB, (p_UCHAR)"", 0, hVocX);
	D_InitGNT(hDB, hVocX);
	LoopBeg();
	while (D_GetNextTail((p_UCHAR) szBuff, &nWeight, hVocX))
	{
		if (!hVocL)
		{
			Reverse(szBuff);
		}
		LoopCnt(szBuff);
		fprintf(f, "%-16s %d\n", szBuff, nWeight);
	}
	D_DestroyHDB(hDB, hVocX);
	fclose(f);
	return;
}


BOOL VocLoadBin(_HD *phVocL, _HD *phVocR)
{
	char   zExtn[16];
	_HFILE hFile;
	_HD    hVocX;
	BOOL   bFlg = FALSE;
	BOOL   bLft = FALSE;
	BOOL   bRgt = FALSE;
	LPSTR  lpFile;
	_HD     hVocL, hVocR;

	*phVocL = *phVocR = NULL;

	SetName("");
	lpFile = VocNameForLoad(hWnd);
	if (!lpFile)
	{
		return(FALSE);
	}


	DirGetExtn(zExtn, lpFile);

	if (!lstrcmp(zExtn, ".l_r"))
	{
		bLft = TRUE;
	}
	if (!lstrcmp(zExtn, ".l_R"))
	{
		bLft = TRUE;
	}
	if (!lstrcmp(zExtn, ".L_r"))
	{
		bLft = TRUE;
	}
	if (!lstrcmp(zExtn, ".L_R"))
	{
		bLft = TRUE;
	}

	if (!lstrcmp(zExtn, ".r_l"))
	{
		bRgt = TRUE;
	}
	if (!lstrcmp(zExtn, ".r_L"))
	{
		bRgt = TRUE;
	}
	if (!lstrcmp(zExtn, ".R_l"))
	{
		bRgt = TRUE;
	}
	if (!lstrcmp(zExtn, ".R_L"))
	{
		bRgt = TRUE;
	}

	if (!bLft && !bRgt)
	{
		MessageBox(hWnd, "Can't recognize dictionary type...", NULL, MB_OK | MB_ICONSTOP);
		return(FALSE);
	}
	bFlg = bRgt;

	if ((hFile = HWRFileOpen((_STR) lpFile, HWR_FILE_RDONLY, HWR_FILE_EXCL)) == _NULL)
	{
		MessageBox(hWnd, "Can't open file for read dictionary...", NULL, MB_OK | MB_ICONSTOP);
		return(FALSE);
	}
	hVocX = D_Dopen(hFile);
	if (!hVocX)
	{
		MessageBox(hWnd, "Can't open dictionary...", NULL, MB_OK | MB_ICONSTOP);
		HWRFileClose(hFile);
		return(FALSE);
	}
	HWRFileClose(hFile);

	if (bFlg)
	{
		hVocL = 0;
		hVocR = hVocX;
	}
	else
	{
		hVocL = hVocX;
		hVocR = 0;
	}
	*phVocL = hVocL;
	*phVocR = hVocR;
	return(TRUE);
}

void VocSaveBin(_HD hVocL, _HD hVocR)
{
	_HFILE hFile;
	LPSTR  lpFile = VocNameForSave(hWnd);

	if (!lpFile)
	{
		return;
	}

	DelPoint(lpFile);
	lstrcat(lpFile, ".l_r");
	if ((hFile = HWRFileOpen((_STR) lpFile, HWR_FILE_NORMAL, HWR_FILE_TRUNC)) == _NULL)
	{
		MessageBox(hWnd, "Can't open file for writing...", NULL, MB_OK | MB_ICONSTOP);
		return;
	}
	if (!D_Dsave(hFile, hVocL))
	{
		MessageBox(NULL, "Can't save dictionary...", NULL, MB_OK | MB_ICONSTOP);
		HWRFileClose(hFile);
		return;
	}
	HWRFileClose(hFile);

	DelPoint(lpFile);
	lstrcat(lpFile, ".r_l");
	if ((hFile = HWRFileOpen((_STR) lpFile, HWR_FILE_NORMAL, HWR_FILE_TRUNC)) == _NULL)
	{
		MessageBox(NULL, "Can't open file for writing...", NULL, MB_OK | MB_ICONSTOP);
		return;
	}
	if (!D_Dsave(hFile, hVocR))
	{
		MessageBox(NULL, "Can't save dictionary...", NULL, MB_OK | MB_ICONSTOP);
		HWRFileClose(hFile);
		return;
	}
	HWRFileClose(hFile);
}

int  GetC(FILE *f)
{
	int c;

	c = fgetc(f);

	bLine = (c == '\n');

	if (c == '\t')
	{
		return(' ');
	}
	if (c == '\n')
	{
		return(' ');
	}
	return(c);
}

int LoadStr(FILE *f, LPSTR zBuff, int n)
{
	int c;
	int i;

	c = GetC(f);
	while (c == ' ')
	{
		c = GetC(f);
	}

	if (c == EOF)
	{
		return(0);
	}

	for (i = 0; c != ' ' && i < n - 1; i++, c = GetC(f))
	{
		/*
		if( !IsCharAlphaNumeric((char)c) )
		{
		while(c!=' ' && c!=EOF) c=GetC(f);
		return(-1);
		}
		*/
		zBuff[i] = c;
		zBuff[i + 1] = 0;
	}
	if (bLine)
	{
		ungetc('\n', f);
	}

	if (i >= n - 1)
	{
		return(-1);
	}

	return(1);
}

int LoadNum(FILE *f, LPSTR zBuff, int n)
{
	int c;
	int i;

	c = GetC(f);
	while (c == ' ')
	{
		if (bLine)
		{
			zBuff[0] = '0';
			zBuff[1] = 0;
			return(1);
		}
		c = GetC(f);
	}

	if (c == EOF)
	{
		return(0);
	}

	for (i = 0; c != ' ' && i < n - 1; i++, c = GetC(f))
	{
		if (!IsCharAlphaNumeric((char) c))
		{
			while (c != ' ' && c != EOF)
			{
				c = GetC(f);
			}
			return(-1);
		}
		if (IsCharAlpha((char) c))
		{
			while (c != ' ' && c != EOF)
			{
				c = GetC(f);
			}
			return(-1);
		}
		zBuff[i] = c;
		zBuff[i + 1] = 0;
	}
	return(1);
}

LPSTR DirGetExtn(LPSTR zBuff, LPSTR zDir)
{
	int n = lstrlen(zDir);
	int i;

	for (i = n - 1; i >= 0; i--) if (zDir[i] == '.')
		{
			lstrcpy(zBuff, zDir + i);
			return(zBuff);
		}
	zBuff[0] = 0;
	return(zBuff);
}

// --------------------------------------------------------------

static HCURSOR hcurSave;
static BOOL    bLoop;
static long    lLoop;
static int     n;

void LoopBeg()
{
	hcurSave = SetCursor(LoadCursor(NULL, IDC_WAIT));
	bLoop = TRUE;
	lLoop = 0L;
	n = 0;
}

void LoopCnt(LPSTR lpWord)
{
	char szBuff[256];

	lLoop++;
	n++;

	if (n == 100)
	{
		n = 0;
	}
	else
	{
		return;
	}

	wsprintf(szBuff, "%ld: %s", lLoop, lpWord);
	SetWindowText(GetDlgItem(hWnd, IDC_VOC_INFO), (LPSTR) szBuff);
}

void LoopEnd()
{
	char szBuff[256];

	if (!bLoop)
	{
		return;
	}
	else
	{
		bLoop = FALSE;
	}

	SetCursor(hcurSave);
	SetWindowText(GetDlgItem(hWnd, IDC_VOC_INFO), (LPSTR)"");

	wsprintf(szBuff, "%ld words processed...", lLoop);
	MessageBox(hWnd, szBuff, "Vocabulary Service", MB_OK || MB_ICONINFORMATION);
}

