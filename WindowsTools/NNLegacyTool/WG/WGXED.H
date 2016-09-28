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

/* ************************************************************************************* * /
/* *    PhatWare WritePad handwriting recognition engine configurator                  * */
/* *    Copyright (c) 1997-2014 PhatWare(r) Corp. All rights reserved.                 * */
/* ************************************************************************************* */

/* ************************************************************************************* *
*
* File: wgxed.h
*
* Unauthorized distribution of this code is prohibited.
* Contractor/manufacturer is PhatWare Corp.
* 1314 S. Grand Blvd. Ste. 2-175 Spokane, WA 99202
*
* ************************************************************************************* */


#define ID_XRSTRING       2000
#define ID_XRKEYBOARD     2001
#define ID_REPLACE        2002
#define ID_DELETE         2003
#define ID_INSERT         2004
#define ID_XRCOLORHEIGHT  2006
#define ID_ATTRIB         2008
#define ID_PENALTIES      2009
#define ID_SHIFT          2010
#define ID_ORIENT         2011
#define ID_LINK           2012
#define CT1               2013
#define CT2               2014
#define CT3               2015
#define CT4               2016
#define CT5               2017
#define IDC_INDEX         2018

#define MYSTYLE WS_BORDER | WS_CHILD | WS_CLIPSIBLINGS
LRESULT CALLBACK xedDialogProc(HWND hWnd, UINT message,
                               WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK xedXrKeyboard(HWND hWnd, UINT message,
                               WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK xedXrEditString(HWND hWnd, UINT message,
                                 WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK xedXrColorHeight(HWND hWnd, UINT message,
                                  WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK xedXrAttributes(HWND hWnd, UINT message,
                                 WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK xedXrPenalty(HWND hWnd, UINT message,
                              WPARAM wParam, LPARAM lParam);
void  FAR xedCreateEditDialog(HWND hWnd);
