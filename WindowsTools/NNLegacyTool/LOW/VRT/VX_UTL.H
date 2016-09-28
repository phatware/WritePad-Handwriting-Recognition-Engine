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
* File: vx_utl.h
*
* Unauthorized distribution of this code is prohibited.
* Contractor/manufacturer is PhatWare Corp.
* 1314 S. Grand Blvd. Ste. 2-175 Spokane, WA 99202
*
* ************************************************************************************* */

// ********** Functions *****************************************
_SHORT CopyTrace(p_SHORT x, p_SHORT y, _SHORT n, p_SHORT xReal, p_SHORT yReal, _SHORT nReal, _LONG nLength);
_LONG  CalcLngSq(_SHORT x1, _SHORT y1, _SHORT x0, _SHORT y0);
_SHORT CalcAngle(_SHORT x0, _SHORT y0, _SHORT x1, _SHORT y1);
_SHORT CalcAng12(_SHORT x0, _SHORT y0, _SHORT x1, _SHORT y1, _SHORT x2, _SHORT y2);
_SHORT CalcDir12(_SHORT x0, _SHORT y0, _SHORT x1, _SHORT y1, _SHORT x2, _SHORT y2);
_BOOL  CalcDirec(_SHORT x0, _SHORT y0, _SHORT x1, _SHORT y1, _SHORT x2, _SHORT y2);

_DOUBLE HSin(_SHORT);
_DOUBLE HCos(_SHORT);


#define TIME_BEG 1
#define TIME_STP 2
#define TIME_CNT 3
#define TIME_END 4

_VOID   DTime(int nDir, char *lpszMsg);


_VOID ShowReady(p_SHORT pReady, p_SHORT X1, p_SHORT X2, p_SHORT x, p_SHORT y, _SHORT n);
_VOID ShowAngls(p_SHORT Ready, p_SHORT Direc, p_SHORT Angl1, p_SHORT Angl2, p_SHORT Angl3, p_SHORT AnglT, _SHORT N);

p_SHORT PntGetX();
p_SHORT PntGetY();
_SHORT PntGetN();
_VOID   PntFree();
_SHORT  PntSkip(p_SHORT x, p_SHORT y, _SHORT n, _SHORT i);
_BOOL   PntCopy(p_SHORT xTraj, p_SHORT yTraj, _SHORT nTraj);
