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

#define   PS_INI_NAME     "ps.ini"

typedef int (WINAPI *RCBACCESS)(void FAR *, int, int, UINT, ULONG);

BOOL FAR recRecognizeTrace(LP_TRACE lpTrace, LPSTR lpSel,
                           BOOL UseUpper, BOOL WordCut, int WordOffset);
#ifdef _PENWIN
BOOL  FAR recRecognizeData(HWND hWnd, HPENDATA hpendata,
                           LPGUIDE lpg, BOOL UseUpperLevel, BOOL TabletCoord);
void  FAR recDone(HREC hrec);
BOOL  FAR recLoadRecognizer(LPSTR lpszRecname);
#else
BOOL     FAR recLoadRecognizerNT(LPSTR lpszRecname);
BOOL     FAR recRecognizeDataNT(HWND hWnd, BOOL UseUpperLevel, BOOL TabletCoord);
BOOL     FAR recRecognizeCommData(p_TRACEDATA lpCommTrace);
BOOL     FAR recGetCapsAndAmnisty(LPINT pData);
p_rec_info_type FAR recGetRCinfo(void);
HINSTANCE FAR  recGetDLLHandle(void);
#endif

typedef BOOL(FAR *LoadRec)(LPSTR);


LPGUIDE  FAR recGetGuidesPtr(void);
HANDLE FAR recInit(LPSTR lpszRecname);
void   FAR recConfig(void);
BOOL   FAR recChangeIniFile(HWND hWnd, LPSTR lpszIniName, LPSTR lpActualIniName, BOOL ReloadDTE);
int    FAR recGetRecPathName(LPSTR buffer);
int    FAR recSetRecPathName(LPSTR buffer);
void   FAR recSetPsIniName(LPSTR buffer);
void   FAR recGetPSIniName(LPSTR buffer);

void   FAR recSetIniFileName(LPSTR IniName, LPSTR RecName);

LPSTR IniLoad(LPSTR section, LPSTR entry, LPSTR inifile);
LPSTR recFindInIni(LPSTR pini, LPCSTR filename, LPSTR fullname);
BOOL  FAR recGetIniFileName(HWND hWnd, LPSTR szIniName);
void  FAR recClose(void);

// AVP 3/31/94
HINSTANCE Q_LoadLibrary(LPCSTR libname);
BOOL      Q_FreeLibrary(HINSTANCE hdll);
FARPROC   Q_GetProcAddress(HMODULE hmod, LPCSTR fname);

