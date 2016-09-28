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
* File: wlink.h
*
* Unauthorized distribution of this code is prohibited.
* Contractor/manufacturer is PhatWare Corp.
* 1314 S. Grand Blvd. Ste. 2-175 Spokane, WA 99202
*
* ************************************************************************************* */

BOOL    wLinkRegister(LPSTR lpszEntry);
LRESULT wLinkSendMessage(LPSTR lpszEnt, UINT uMsg, WPARAM wParam, LPARAM lParam);
HWND    wLinkGetWindow(LPSTR lpszEnt);
void    wLinkSaveName(LPSTR lpszEntry, LPSTR lpszName);
LPSTR   wLinkLoadName(LPSTR lpszEntry, LPSTR lpszBuff, short nSize);
BOOL    wLinkTest(UINT uMsg, ATOM aEnt);

#define SD_SND_BUFF	1000
#define SD_SET_NAME 1001
#define SD_SET_ORGN 1002
#define SD_SET_SIZE 1003
#define SD_SET_TOOL 1004
#define SD_DRW_MOVE 1005
#define SD_DRW_LINE 1006
#define SD_CLR_DATA 1007
#define SD_DRW_DATA 1008
#define SD_DRW_ELEM 1009
#define SD_SND_DATA 1010

#define SD_TYPE_IMAGE  1

typedef struct _Data
{
	WORD  cmd;
	short   x;
	short   y;
}	Data;

#define BUFSIZE	1024
#define wLinkBufferName "wLinkBufferName"

short SD_Create(LPSTR szName);
void  SD_Update(int tool);
void  SD_Delete(int tool);
void  SD_Select(int t, int c);
void  SD_MoveTo(int x, int y);
void  SD_LineTo(int x, int y);
void  SD_DrawXr(int type, int ornt);
void  SD_Window(int x, int y, int dx, int dy);
void  SD_SendDT(int type, void *lpData, int size);
