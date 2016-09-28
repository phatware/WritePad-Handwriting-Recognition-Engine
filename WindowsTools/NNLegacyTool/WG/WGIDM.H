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

/* ************************************************************************************* */
/* *    PhatWare WritePad handwriting recognition engine configurator                  * */
/* *    Copyright (c) 1997-2014 PhatWare(r) Corp. All rights reserved.                 * */
/* ************************************************************************************* */

/* ************************************************************************************* *
*
* File: wgidm.h
*
* Unauthorized distribution of this code is prohibited.
* Contractor/manufacturer is PhatWare Corp.
* 1314 S. Grand Blvd. Ste. 2-175 Spokane, WA 99202
*
* ************************************************************************************* */


// MAIN menu IDM

#define IDM_OPEN_TAP            100
#define IDM_SAVE_TAP            101
#define IDM_OPEN_INKOUTPUT      102
#define IDM_EXIT                103
#define IDM_CLOSE_DOC           104
#define IDM_INSTALL_REC         105
#define IDM_RECORD_TAP          106
#define IDM_FILE_1              113
#define IDM_FILE_2              114
#define IDM_FILE_3              115
#define IDM_FILE_4              116
#define IDM_MAIL_TAP            117
#define IDM_MAIL_SCREEN         118
#define IDM_VOC                 120
/* Is not real Menu ID - for passing command line parameters only */
#define IDM_BATCH               199

//CHE:
#define IDM_PROTOMIX            401
#define IDM_RELOAD_DDE          402

//??IB
#define IDM_OPEN_LRN            201
#define IDM_CASCADE_DOC         1000
#define IDM_ARRANGE_DOC         1001
#define IDM_CLOSEALL_DOC        1002
#define IDM_TILE_DOCV           1003
#define IDM_TILE_DOCH           1004

//  TAP  IDM
#define IDM_REC_RECOGNIZE       2000
#define IDM_REC_CONFIG          2001
#define IDM_REC_UPPER           2002
#define IDM_REC_SAVEDESFILE     2004
#define IDM_INK_RECORDING       2005
#define IDM_STOP_RECORDING      2006
#define IDM_REC_WORDCUT         2007
#define IDM_SAVE_TRAJECTORY     2008
#define IDM_REC_BATCH           2009
#define IDM_REC_RESULT          2010
//  draw ink  IDM
#define IDM_ZOOM_IN             3000
#define IDM_ZOOM_OUT            3001
#define IDM_NO_ZOOM             3002
#define IDM_PREV_WORD           3003
#define IDM_NEXT_WORD           3004
#define IDM_ZOOM_INCTL          3005
#define IDM_ZOOM_OUTCTL         3006

//  text output  IDM
#define IDM_SELECT_FONT         9000
#define IDM_CLEAR_WINDOW        9001
#define IDM_SET_WINDOW_BEGIN    9002
#define IDM_BUFFER_INPUT        9003

// utilities
#define IDM_COR_MATRIX          4000
#define IDM_STOP_DEBUGGING      4001
#define IDM_EDIT_INIFILE        4002
#define IDM_V_FUNCTION          4003
#define IDM_XREDIT              4004
#define IDM_V2_FUNCTION         4006
#define IDM_WORD_LAYOUT         4009
#define IDM_CLEAR_TEXT          4013
#define IDM_READ_WACOM          4014
#define IDM_STOP_READ_WACOM     4015
#define IDM_SET_PORTS           4016
#define IDM_COMPRESS            4017
#define IDM_DECOMPRESS          4018
#define IDM_LOWDBG_BREAKS       4019
#define IDM_LOWDBG_CROSS_G      4020
#define IDM_LOWDBG_LK_BEGIN     4021
#define IDM_LOWDBG_LK_NEXT      4022

#define IDM_PREFERNCES          5000
#define IDM_DEBUG               6000
#define IDM_SHOWGRAPH           6001

/* Is not real Menu ID - fake IDM - proceed with or start recognizing after
   Debug level was changed in tool bar control */
#define IDM_PROCEED             6000
#define IDM_SEARCH              7000
#define IDM_SLOW_DRAW           8000
// shared child windows IDM

#define IDM_SCROLLV             9996
#define IDM_SCROLLH             9997

#define IDM_CREATE_INKWINDOWS   9999


#define IDM_FIRSTCHILD          10000 // should be greater then max IDM above

#define WNDMAIN_VERSION         10001
