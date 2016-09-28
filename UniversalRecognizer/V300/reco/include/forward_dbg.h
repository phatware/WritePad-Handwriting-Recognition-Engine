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


#if  PG_DEBUG
 #include <stdio.h>
 #include "pg_debug.h"

 _VOID  GetTraceBox ( p_SHORT xArray, p_SHORT yArray,
                      _SHORT iLeft, _SHORT iRight,
                      p_RECT pRect );

  static _SHORT  DBG_iFrstStrk = 0;
  static _SHORT  DBG_iCurStrk  = 0;

#define  DBG_PUTWIDEPIXEL(iPoint,color)                 \
  {                                                     \
    draw_arc( (color), xArray, yArray,                  \
              (iPoint), (iPoint) );                     \
  }

#define  DBG_CLR_PLATE      {}

#define  DBG_SET_CURSTROKE  {DBG_iCurStrk = iStrk;}

             /*  AltX ends debugging painting: */
#define  DBG_BRKEYW(S)                                  \
  {                                                     \
    if  ( brkeyw(S) == 0x2d00 )                         \
      DBG_iFrstStrk = 9999;                             \
  }

#define  DBG_MESSAGE(S)                              \
 {                                                   \
  if  (   mpr >= 100                                 \
       && DBG_iCurStrk >= DBG_iFrstStrk )            \
    {                                                \
      _CHAR msgTMP[128] = "\n";                      \
      HWRStrCat( msgTMP, (S) );                      \
      DBG_BRKEYW(msgTMP);                            \
    }                                                \
 }

#define  DBG_GET_DEMO_BORDERS(nWorkPoints)              \
 if  ( mpr >= 100 )                                     \
  {                                                     \
    _RECT  DBGbox;                                      \
    GetTraceBox(xArray,yArray,1,(nWorkPoints)-1,        \
                &DBGbox );                              \
    draw_init( 0,0,0,DBGbox.right,DBGbox.bottom,        \
               xArray, yArray, (nWorkPoints) );         \
  }

#define  DBG_SHOW_PART(iBeg,iEnd,color)                  \
 if  ( mpr >= 100 )                                      \
  {                                                      \
    draw_arc( (color), xArray, yArray, (iBeg), (iEnd) ); \
  }

#define  DBG_SHOW_TRACE                                  \
 if  ( mpr >= 100 )  {                                   \
   DBG_SHOW_PART(0,nWorkPoints-1,EGA_WHITE)              \
   DBG_BRKEYW("\nAt FRM_WORD beg...");                   \
 }

#define  DBG_INQUIRE_FIRST_STROKE                                \
  if  ( mpr >= 100 )  {                                          \
    _CHAR  szTmp[80];                                            \
    HWRTextSPrintf (szTmp,"\nFirst stroke to stop (%d) : ",DBG_iFrstStrk);  \
    if  ( InqString(szTmp,szTmp)==1 )                            \
      DBG_iFrstStrk = HWRAtoi(szTmp);                            \
    if  ( DBG_iFrstStrk >= nStrokes )                            \
     DBG_iFrstStrk = nStrokes - 1;                               \
    CloseTextWindow();                                           \
  }

#define  DBG_SHOW_WORDBEG                                    \
  if  ( mpr >= 100 )  {                                      \
    draw_arc( EGA_YELLOW, xArray, yArray,                    \
              strkCur->iBeg, strkCur->iEnd );                \
    delay(300);                                              \
    draw_arc( EGA_GREEN, xArray, yArray,                     \
              strkCur->iBeg, strkCur->iEnd );                \
    delay(300);                                              \
    draw_arc( EGA_YELLOW, xArray, yArray,                    \
              strkCur->iBeg, strkCur->iEnd );                \
    delay(300);                                              \
    draw_arc( EGA_GREEN, xArray, yArray,                     \
              strkCur->iBeg, strkCur->iEnd );                \
    DBG_BRKEYW("\nNew WordBrk found");                       \
  }

#define  DBG_SHOW_NEXT_STROKE                                \
 if  ( mpr >= 100 )  {                                       \
    draw_arc( EGA_YELLOW, xArray, yArray,                    \
              strkCur->iBeg, strkCur->iEnd );                \
    DBG_MESSAGE("The next stroke drawn ...");                \
 }

#define  DBG_END_FRM_WORD {if(mpr>=100) mpr=-6;}

#else  /*!PG_DEBUG*/

#define  DBG_PUTWIDEPIXEL(i,clr)   {}
#define  DBG_CLR_PLATE             {}
#define  DBG_GET_DEMO_BORDERS(nWorkPoints)      {}
#define  DBG_SHOW_PART(iBeg,iEnd,color)         {}
#define  DBG_SHOW_TRACE            {}
#define  DBG_INQUIRE_FIRST_STROKE  {}
#define  DBG_SHOW_WORDBEG          {}
#define  DBG_SHOW_NEXT_STROKE      {}
#define  DBG_SET_CURSTROKE         {}
#define  DBG_MESSAGE(S)            {}
#define  DBG_END_FRM_WORD          {}

#endif /*!PG_DEBUG*/
