/***************************************************************************************
 *
 *  WRITEPAD(r): Handwriting Recognition Engine (HWRE) and components.
 *  Copyright (c) 2001-2017 PhatWare (r) Corp. All rights reserved.
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

#ifndef __Reco_Defs_h__
#define __Reco_Defs_h__

#define IMAGE_SUPPORT           1
#define TEXT_SUPPORT            1
#define HW_RECINT_UNICODE       1           // NOTE: define to use Unicode (UTF-16)

#ifndef WIN32
#define GetRValue(rgb)          ((float)((rgb)&0xFF)/255.0)
#define GetGValue(rgb)          ((float)(((rgb)>>8)&0xFF)/255.0)
#define GetBValue(rgb)          ((float)(((rgb)>>16)&0xFF)/255.0)
#endif // WIN32

#define GetAValue(rgb)          ((float)(((rgb)>>24)&0xFF)/255.0)
#define RGBA(r,g,b,a)           ((COLORREF)(((unsigned char)(r)|((unsigned int)((unsigned char)(g))<<8))|(((unsigned int)(unsigned char)(b))<<16)|(((unsigned int)(unsigned char)(a))<<24)))
#define CCTB(cc)                ((unsigned char)(cc * (float)0xFF))


/* ------------------------- Structures ------------------------------------- */

typedef void * RECOCTX;						/* Type of handle of recognizer context */
typedef void * RECOHDICT;					/* Type of handle of user dictionary handle */


#if HW_RECINT_UNICODE
typedef unsigned short UCHR;
typedef const unsigned short CUCHR;
#else  // HW_RECINT_UNICODE
typedef char UCHR;
typedef const char CUCHR;
#endif // HW_RECINT_UNICODE

typedef int (RECO_ONGOTWORD)( const UCHR * szWord, void * pParam );
typedef RECO_ONGOTWORD * PRECO_ONGOTWORD;

#endif // __Reco_Defs_h__
