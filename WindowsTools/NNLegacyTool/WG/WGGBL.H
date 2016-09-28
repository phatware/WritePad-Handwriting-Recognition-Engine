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

#ifdef MAIN
#define global
#else
#define global extern
#endif

#define VER_STRING "WritePad Configurator"

global HINSTANCE   hInst ;
global HWND     hMainWnd;
global HWND     hWndClient;
global HCURSOR  hPenCursor;

global HMENU    hMainMenu;
global HMENU    hMainMenuWnd;
global HMENU    hTAPMenu;
global HMENU    hTAPMenuWnd;
global HMENU    hInkOutputMenu;
global HMENU    hInkOutputMenuWnd;

#define DEF_DEBUGLEVEL	-10
global short    DebugLevel;

global short    LookLevel;
//global BOOL     fCmdLine;
global char     szInkWindowName[128];

global BOOL     bSaveDesFile
#ifdef MAIN
    = FALSE
#endif
      ;

global BOOL     gWordCut
#ifdef MAIN
    = FALSE
#endif
      ;

global BOOL     gBatchProccesing
#ifdef MAIN
    = FALSE
#endif
      ;

global BOOL     gBatchTapWordProccesing
#ifdef MAIN
    = FALSE
#endif
      ;

global BOOL     gPenLabBatchMode
#ifdef MAIN
    = FALSE
#endif
      ;

global BOOL     gUseUpper
#ifdef MAIN
    = FALSE
#endif
      ;

global WORD     WaitFlag
#ifdef MAIN
    = 0
#endif
      ;

global WORD     WaitInput
#ifdef MAIN
    = 0
#endif
      ;

global WORD     WaitOutput
#ifdef MAIN
    = 0
#endif
      ;

global HWND     hServerWnd            // DDE server handle
#ifdef MAIN
    = NULL
#endif
      ;

global HWND     hLastDebugWnd         // last active output window
#ifdef MAIN
    = NULL
#endif
      ;

global HWND     hLastTAPWnd           // last TAP file window
#ifdef MAIN
    = NULL
#endif
      ;

global BOOL     gIsRecognizing
#ifdef MAIN
    = FALSE
#endif
      ;

global UINT     modeStart
#ifdef MAIN
    = FALSE
#endif
      ;

#define WNDMAIN_CLASSNAME   "WGWClass"
#define WNDTAP_CLASSNAME    "TAPClass"
#define WNDTOOL_CLASSNAME   "TOOLBARClass"
#define WNDDBG_CLASSNAME    "INKTEXTClass"
#define WNDTEXT_CLASSNAME   "TEXTClass"
#define WNDDRAW_CLASSNAME   "INKClass"

#define MNUMAIN_NAME    "WGMenu"
#define MNUTAP_NAME     "TAPMenu"
#define MNUDBG_NAME     "INKOUTPUTMenu"
#define ACCMAIN_NAME    "WG"

#define ICOMAIN_NAME    "WGIcon"
#define ICOTAP_NAME     "WGTAPIcon"
#define ICODBG_NAME     "WGDBGIcon"

#define WNDMAIN_TITLE   "WritePad Configurator"

#define AM_MDISETMENU   WM_USER+300
#define AM_GETRECNAME   WM_USER+350
#define AM_BATCH        WM_USER+330
#define AM_PARSECMDLINE WM_USER+200


/* ?? IB some learning */
#define ICOLRN_NAME      "learnico"

#define WNDLRN_CLASSNAME "learnClass"

#define WNDCHR_CLASSNAME "charClass"

#define IDW_XRLBOX 2345
#define AM_ONCREATE WM_USER+1234

global HWND hLastLearnWnd
#ifdef MAIN
    = NULL
#endif
      ;
/*?? IB*/
global HMENU    hLearnMenu;
global HMENU    hLearnMenuWnd;

/* ??IB */
#define MNULRN_NAME     "LearnMenu"

#define  _HWR_NEW 1

#ifndef _WIN32
#define GET_WM_COMMAND_ID(wp, lp)     (WORD)(wp)
#define GET_WM_COMMAND_HWND(wp, lp)   (HWND)LOWORD(lp)
#define GET_WM_COMMAND_CMD(wp, lp)    HIWORD(lp)
#define ULONG                         unsigned long
#endif
