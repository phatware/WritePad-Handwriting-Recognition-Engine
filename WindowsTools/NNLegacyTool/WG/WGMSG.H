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

#define IDC_WORD        100
#define IDC_DIRECT      101
#define IDC_INVERSE     102
#define IDC_SHOW_COR    103
#define IDC_USE_EXTRA   104
#define IDC_SELLET      105
#define IDC_CAPMODE     106
#define IDC_AMNISTY     107
#define IDC_SPECIALMODE 108

#define IDC_SET_DEFAULT                        109
#define IDC_DX_RATIO_FOR_GU                    110
#define IDC_DY_RATIO_FOR_GU                    111
#define IDC_DX_RATIO_FOR_O                     112
#define IDC_DY_RATIO1_FOR_O                    113
#define IDC_DY_RATIO2_FOR_O                    114
#define IDC_RATIO1_POINTS_IN_CROSS_TO_BE_O     115
#define IDC_RATIO2_POINTS_IN_CROSS_TO_BE_O     116
#define IDC_EDIT1	117
#define IDC_DLT_GAMM_TO_BE_O                   117
#define B_IDLE      0
#define B_BUSY      1

//CHE
#define PROTOMIXING       0
LRESULT msgHWDebug(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) ;
LRESULT msgCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) ;
LRESULT msgCreate(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) ;
LRESULT msgDestroy(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) ;
LRESULT msgClose(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) ;
LRESULT msgUser(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) ;
LRESULT msgSize(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) ;
LRESULT msgMDISETMENU(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT msgPARSECMDLINE(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT msgBatchRecognition(HWND hWnd, LPARAM lParam);

void FAR msgUpdateLevels(void) ;
void FAR msgStartDebug(void) ;
void FAR msgStopDebug(BOOL StopAllInteraction) ;
void FAR msgCancelDebug(void) ;
int  FAR msgStopAndGo(DWORD TimeDelay, int InterruptMode) ;
#ifdef _PENWIN
void FAR msgPassXRData(LPARAM XRline);
#endif
void FAR msgPassMatrixWord(void);
void FAR msgWordLayOut(LPSTR lpParam);
int  FAR msgGetWordTraj(LPSTR lParam);

int   FAR msgGetWordCut(void);
int   FAR msgSetWordCut(int OldWordCut);
int   FAR msgGetSureLevel(void);
void  FAR msgSetSureLevel(int NewLevel);

HWND IsRecWndExist(HINSTANCE hInst, LPSTR lpCmdLine);

LRESULT CALLBACK msgMatrixDlgBox(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) ;
LRESULT CALLBACK msgLayoutDlgBox(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) ;

int  FAR msgDDECommandLine(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) ;
