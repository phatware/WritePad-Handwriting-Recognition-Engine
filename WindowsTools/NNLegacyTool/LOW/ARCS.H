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

#ifndef  ARCS_H_INCLUDED
#define  ARCS_H_INCLUDED

/* #define  D_ARCS    */           /* if you want realy arcs.....         */

/***********************  Owner debug defines  ****************************/
#define     ANDREI_DEB
/**************************************************************************/

/***********************  Owner masked defines  ***************************/
/**************************************************************************/


/********************************************************/

_VOID   InitElementSDS(p_SDS pSDS);

_SHORT  SlashArcs(low_type _PTR  pLowData, _INT iBeg, _INT iEnd);

_VOID   InitSDS(_SDS asds [], _SHORT _PTR lsds, _SHORT n);

_BOOL   RelHigh(p_SHORT y, _INT begin, _INT end,
                _INT height [], p_SHORT plowrelh, p_SHORT puprelh);


_VOID   DotPostcrossModify(low_type _PTR  pLowData);
/********************************************************/

#define    SHORT_BASE                100       /* Short scaling base . */
#define    LONG_BASE                 100L      /* Long scaling base .  */
#define    MAX_NO_VERT               1500L
#define    DEF_MINLENTH              15L       /* Default minimum  sig- */
/* nificant lenth .     */
#define    DEF_MINLENTH_S            10


#define    MIN_NO_HOR                4


/********************************************************/


/********************************************************/
/*           Arcs description structures:               */
/********************************************************/



#ifdef D_ARCS

typedef struct
{
	_SHORT    iBeg    ;      /*  Index of the 1st point                 */
	_SHORT    iEnd    ;      /*  Index of the last point                */
	_SHORT    nLength ;      /*  The length of the chord at this arc    */
	_SHORT    nCurv   ;      /*  Curvature of the arc, measured with    */
	/* "CurvMeasure" function.  See comments   */
	/*  at the head of that function in module */
	/*  LOW_UTIL.C .                           */
	_RECT     box     ;      /*  The box containing the arc.            */
	_ARC_TYPE  type   ;      /*  The interpretation of the arc.         */
}
ARC_DESCR, _PTR p_ARC_DESCR     ;

typedef struct
{
	p_ARC_DESCR  pArcData       ;
	_SHORT       arc_buf_Len    ;
	_SHORT       LenArcData     ;
}
ARC_CONTROL, _PTR p_ARC_CONTROL ;

/********************************************************/

_SHORT  Prepare_Arcs_Data( p_ARC_CONTROL pArcControl  ) ;

_VOID   Dealloc_Arcs_Data ( p_ARC_CONTROL pArcControl ) ;

_SHORT  ArcRetrace( low_type _PTR p_low_data , p_ARC_CONTROL pArcControl );

_SHORT  Arcs( low_type _PTR low_data, p_ARC_CONTROL pArcControl ) ;

/********************************************************/

#define    LEN_ARC_BUFFER            50

/********************************************************/

#endif    /* D_ARCS */

#endif    /* ARCS_H_INCLUDED */

