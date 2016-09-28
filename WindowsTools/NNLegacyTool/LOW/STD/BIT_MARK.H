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

#include "ams_mg.h"

#if defined( FOR_GERMAN) || defined (FOR_FRENCH) || defined(FOR_INTERNATIONAL)

/*************************************************************************/


#ifdef  FILE_TESTING
#include   <stdio.h>
#endif

/*-----------------------------------------------------------------------*/

#define    MAX_MARK                      10

#define    BITS_FOR_SINGLE_MARK          4

#define    BIT_MARK_MAX                  15

#define    SCALE_Y_DIMENSION             24

#define    SCALE_X_ELS_DIMENSION         18
#define    SCALE_Y_ELS_DIMENSION         15

#define    SCALE_X_STR_DIMENSION         23
#define    SCALE_Y_STR_DIMENSION         12

#define    SCALE_X_DOT_DIMENSION         15
#define    SCALE_Y_DOT_DIMENSION         11

#define    MAX_BOX_SCALE                 SCALE_Y_DIMENSION

#define    SCALE_EXT_DIMENSION           4
#define    SCALE_INT_DIMENSION           2
#define    SCALE_POS_DIMENSION           3

#define    SCALE_BOX_EXT_UML_STR_DIM     2
#define    SCALE_BOX_INT_ELS_UML_DIM     2

#define    INTERSECTED                   1
#define    ISOLATE                       0

#define    END_LAST                      0
#define    MID_LAST                      1
#define    COMMON                        2

/*-----------------------------------------------------------------------*/

typedef struct
{
unsigned int    single_mark   :
	BITS_FOR_SINGLE_MARK  ;
} BIT_MARK ,
_PTR  p_BIT_MARK ;


typedef struct
{
	p_BIT_MARK  pBitMarkTable ;

	p_UCHAR     pXScale       ;
	_SHORT      mColumns      ;
	p_UCHAR     pYScale       ;
	_SHORT      nLines        ;
}
BIT_MARK_TABLE_CONTROL,
_PTR   p_BIT_MARK_TABLE_CONTROL ;


/**************************************************************************/


_SHORT  GroupsSpeclBegProect( low_type _PTR  pLowData , _SHORT  numGroup );

_SHORT  MarkPCounter( low_type _PTR  pLowData , p_SPECL  pSpecl ,
                      _UCHAR   MarkName  )  ;

_VOID   GetBoxMarks( p_UM_MARKS  pSpaceMarks , _SHORT dX , _SHORT dY  ) ;

_SHORT  FetchTableNumber( _SHORT dX, p_UCHAR Scale, _SHORT ScaleDimension );

_SHORT  WriteUmlData ( p_UM_MARKS_CONTROL  pUmMarksControl  ,
                       p_UM_MARKS          pUmSpaceMarks    )  ;

_VOID   UmIntersectBuild ( low_type _PTR   pLowData )  ;

_VOID   UmIntersectDestroy ( low_type _PTR  pLowData, _SHORT  UmNumGroup ) ;
_SHORT  InterMarks( _CHAR dN , p_BIT_MARK  pInterTable ) ;

_SHORT  GetMarks( p_BIT_MARK_TABLE_CONTROL  pMarksTableControl ,
                  _SHORT dX ,  _SHORT dN   ) ;

_BOOL   ShapeFilter  ( low_type _PTR  pLowData ,
                       _SHORT   iMin0 , _SHORT  iMax , _SHORT iMin1   ) ;

_BOOL   CheckGroup   ( low_type _PTR  pLowData , _SHORT  GroupNumber ) ;

_SHORT  CheckPosition( low_type _PTR  pLowData , _SHORT  GroupNumber ) ;

_VOID   GetPositionMark( low_type _PTR  pLowData ,
                         _SHORT         GroupNumber ,
                         p_UM_MARKS     pUmTmpMarks ) ;

_BOOL   IntersectContains( low_type _PTR  pLowData , _SHORT  NumGroup ) ;
#if PG_DEBUG

_VOID   BoxOutline( p_RECT pBox ) ;

_VOID   HeightEncode( _UCHAR  tmpHeight , p_STR  pCodedHeight ) ;

_VOID   TypeCodedHeight( _UCHAR  tmpHeight ) ;

_VOID   UmInfoTypePaint( low_type _PTR   pLowData  ,
#ifdef  FILE_TESTING
                         FILE       *debUmInfo ,
#endif
                         p_UM_MARKS     pUmTmpMarks, p_UM_MARKS  pBoxMarks ,
                         p_UM_MARKS     pExtMarks  , p_UM_MARKS  pIntMarks ) ;

#endif      /* PG_DEBUG */


#endif /* FOR_FRENCH || FOR_GERMAN */
