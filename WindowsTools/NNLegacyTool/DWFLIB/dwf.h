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
* File: dwf.h
*
* Unauthorized distribution of this code is prohibited.
* Contractor/manufacturer is PhatWare Corp.
* 1314 S. Grand Blvd. Ste. 2-175 Spokane, WA 99202
*
* ************************************************************************************* */

#ifndef __DWF_H__
#define __DWF_H__

#include <stdio.h>
#include <windows.h>

typedef struct tagDIOPOINT
{
	short x;
	short y;
} DIOPOINT, *PDIOPOINT;


typedef	WCHAR *		PSTRING;
#define	Dio_fopen	_wfopen

#define	DWF_MAXSTROKE		100
#define	DWF_MAXPOINT		50000

#define	AT_BEGIN			SEEK_SET
#define	AT_CURRENT			SEEK_CUR
#define	AT_END				SEEK_END

typedef FILE *			HDWF;

typedef	struct tagDWFEXSEGMENT
{
	unsigned short	nCho : 4,
	          nJung : 3,
	          nJong : 3,
	          nCJ : 1,
	          nJJ : 1,
	          nCJJ : 1,
	          nReserved : 1,
	          nGood : 1,
	          nBad : 1;
} DWFEXSEGMENT, *PDWFEXSEGMENT;

typedef struct tagDWFSTRBLOCK
{
	unsigned short	unCode;
	short			nLetters;
	unsigned short	strResult[DWF_MAXSTROKE];
	short			nStrokes;
	short			nPoints;
	DWFEXSEGMENT	udSegment;
	short			nIndex[DWF_MAXSTROKE];
	DIOPOINT		udCoord[DWF_MAXPOINT];
} DWFBLOCK, *PDWFBLOCK;

typedef enum tagDWF_TYPE
{
	DWF_TYPE_CHAR,
	DWF_TYPE_STRING
} DWF_TYPE;

HDWF DWF_Open(PSTRING szFilename, long *plCharNum);
HDWF DWF_Modify(PSTRING szFilename, long *plCharNum);
HDWF DWF_Create(PSTRING szFilename);
long DWF_Seek(HDWF hFile, long lOffset, int nStartPos);
void DWF_Close(HDWF hFile);

BOOL DWF_SetBlockCount(HDWF hFile, long lCharNum);
BOOL DWF_Get(HDWF hFile, PDWFBLOCK pudBlock);
BOOL DWF_Put(HDWF hFile, PDWFBLOCK pudBlock);

#endif
