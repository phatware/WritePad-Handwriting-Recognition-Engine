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

#include <stdlib.h>
#include <stddef.h>

#include "InternalTypes.h"
#include "gestures.h"

#define  STD_SCALE  4

#define  MIN_STD_BOX_SIZE_FOR_GESTURE  40
#define  MAX_STD_BOX_SIZE_FOR_SMALLPT  30

#define  STD_SIZE(std,scale)  \
              ((int)(((long)std*(scale)+STD_SCALE/2)/STD_SCALE))

/************************************************/

//Definitions from CALCMACR.H:

/*  Useful non-portable macros (on other platform they may need */
/* to be redefined, though this is almost unprobable):          */

   /*  The following macros are applicable only to positive */
   /* integer-type values:                                  */

#define  wODD(w)   ((w)&1)
#define  wEVEN(w)  (!wODD(w))

#define  ONE_HALF(X)             ( (X) / 2 )
#define  ONE_THIRD(X)            ( ((X)+1) / 3 )
#define  TWO_THIRD(X)            ( (((X)*2)+1) / 3 )
#define  ONE_FOURTH(X)           ( ((X)+2) / 4 )
#define  ONE_FIFTH(X)            ( ((X) + 2) / 5 )
#define  TWO_FIFTH(X)            ( (((X)*2) + 2) / 5 )
#define  ONE_EIGHTTH(X)          ( ((X) + 4) / 8 )

#define  ONE_NTH(X,N)            ( ((X) + ONE_HALF(N)) / (N) )
#define  MULT_RATIO(X,NMR,DNM)   ( ((X)*NMR + ONE_HALF(DNM)) / (DNM) )

#define  TWO(X)                  ( (X) * 2 )
#define  FOUR(X)                 ( (X) * 4 )
#define  EIGHT(X)                ( (X) * 8 )

#define  THREE_HALF(X)           ( (X) + ONE_HALF(X) )
#define  FOUR_THIRD(X)           ( (X) + ONE_THIRD(X) )
#define  THREE_FOURTH(X)         ( (X) - ONE_FOURTH(X) )
#define  FIVE_FOURTH(X)          ( (X) + ONE_FOURTH(X) )
#define  FOUR_FIFTH(X)           ( (X) - ONE_FIFTH(X) )

#define  THREE(X)                ( (X) + TWO(X) )
#define  FIVE(X)                 ( (X) + FOUR(X) )

#define  TO_TWO_TIMES(X)         {(X) = ((X)*2);}
#define  TO_THREE_TIMES(X)       {(X) = ((X)*3);}

/***************************************/

/*  Useful portable macros: */

#define  DX_RECT(rect)           ((rect).right - (rect).left)
#define  DY_RECT(rect)           ((rect).bottom - (rect).top)

#define  XMID_RECT(rect)         ONE_HALF((rect).left + (rect).right)
#define  YMID_RECT(rect)         ONE_HALF((rect).top + (rect).bottom)

#define  SWAP_SHORTS(w1,w2)      {short w; w=w1; w1=w2; w2=w;}
#define  SWAP_INTS(w1,w2)        {int   w; w=w1; w1=w2; w2=w;}

#define  EQ_SIGN(X,Y)            (((X)>=0) == ((Y)>=0))
#define  TO_ABS_VALUE(X)         {if  ((X) < 0)  (X) = -(X);}

#define  MEAN_OF(X,Y)            ONE_HALF((X) + (Y))

//End Definitions from CALCMACR.H

/*****************************************************************************************/

static _BOOL	IsMonotonous( CGPoint *  pTrace, int iLeft, int iRight , int iXth, int iYth);
static int	GetFarFromChordIndex ( CGPoint *  pTrace, int iLeft, int iRight );
static _BOOL	GestClosedSquare( CGPoint * pTrace, int cTrace, float *plSquare);
static GESTURE_TYPE recognizeGesture( GESTURE_TYPE gtCheck, 
                                     CGPoint * pTrace,
                                     int nPoints,
                                     int nScale,
                                     int nMinLen );

/*****************************************************************************************/

extern "C"
GESTURE_TYPE HWR_CheckGesture( GESTURE_TYPE gtCheck, 
                                   CGStroke stroke, 
                                   int nPoints,
                                   int nScale,
								   int nMinLen )
{
    CGPoint * pTrace = (CGPoint *)HWRMemoryAlloc( (nPoints+1) * sizeof( CGPoint ) );
    if ( NULL == pTrace )
        return GEST_NONE; 

    for ( int i = 0; i < nPoints; i++ )
    {
        pTrace[i] = stroke[i].pt;
    }
	GESTURE_TYPE result = recognizeGesture( gtCheck, pTrace, nPoints, nScale, nMinLen );
    HWRMemoryFree( pTrace );
    return result;
}

static GESTURE_TYPE recognizeGesture( GESTURE_TYPE gtCheck, 
                                     CGPoint * pTrace,
                                     int nPoints,
                                     int nScale,
                                     int nMinLen )
{
    RECT	box;
	int		iTop, iBottom, iLeft, iRight;
	float	dxBox, dyBox;
    int     i;
	int		dxRun, dyRun;
	float	y1, y2;
    int     iFirstExtr = 0, iLastExtr = 0;
	float	x1, x2;
	float	xFirst, yFirst, xLast, yLast;
	int		nRealPts, iFirst, iLast;
    
	if  ( gtCheck == GEST_NONE )
		return GEST_NONE;

	for  ( iFirst=0;  iFirst<nPoints && pTrace[iFirst].y == TRACE_BREAK; iFirst++ )
		;
	for  ( iLast=nPoints-1;  iLast>iFirst && pTrace[iLast].y == TRACE_BREAK;  iLast-- )
		;
	nRealPts = iLast - iFirst + 1;
	
	for(i=iFirst; i<=iLast; i++) 
	{
		if  ( pTrace[i].y < 0 || pTrace[i].x < 0 )
		{
			iLast = i;
			break;
		}
	}
	if ( iLast - iFirst < 5 )	// stroke must be at least 5 pixels long
		return GEST_NONE;
	
	//Prepare work data:
	xFirst = pTrace[iFirst].x;
	yFirst = pTrace[iFirst].y;
	xLast  = pTrace[iLast].x;
	yLast  = pTrace[iLast].y;
	
	box.top = box.bottom = yFirst;
	box.left = box.right = xFirst; 
	iTop = iBottom = iLeft = iRight = iFirst;
	
	for ( i=iFirst+1; i<=iLast; i++ )
	{
        if(pTrace[i].x<box.left)
		{
			box.left = pTrace[i].x;
			iLeft = i;
		}
        else if(pTrace[i].x>box.right)
		{
			box.right = pTrace[i].x;
			iRight = i;
		}
        if(pTrace[i].y<box.top)
		{
			box.top = pTrace[i].y;
			iTop = i;
		}
        else if(pTrace[i].y>box.bottom)
		{
			box.bottom = pTrace[i].y;
			iBottom = i;
		}
	}
	
	dxBox = box.right - box.left;
	dyBox = box.bottom - box.top;
	
	//Count |dx| and |dy| accumulated changes:
	for  ( i=iFirst, dxRun=dyRun=0; i<iLast; i++  ) 
	{
		dxRun += abs( (int)(pTrace[i+1].x - pTrace[i].x) );
		dyRun += abs( (int)(pTrace[i+1].y - pTrace[i].y) );
	}
	
	// GEST_SMALLPT
	if((gtCheck&GEST_SMALLPT) && 
	   nScale>0               &&
	   dxBox<=STD_SIZE(MAX_STD_BOX_SIZE_FOR_SMALLPT,nScale) && 
	   dyBox<=STD_SIZE(MAX_STD_BOX_SIZE_FOR_SMALLPT,nScale))
	{
		return GEST_SMALLPT;
	}
	
	if  (   nScale>0
		 && dxBox<STD_SIZE(MIN_STD_BOX_SIZE_FOR_GESTURE,nScale)
		 && dyBox<STD_SIZE(MIN_STD_BOX_SIZE_FOR_GESTURE,nScale) )
	{
		return GEST_NONE;
	}
	
	// Additional common check for gesture length"
	if  ( nRealPts < 4 )
		return GEST_NONE;
	
	// GEST_SCROLLUP
	if( (gtCheck&GEST_SCROLLUP)             &&
	   dyBox!=0                             &&
	   dxBox<=dyBox/4                       &&
	   box.bottom-yFirst<=dyBox/5           &&
	   yLast-box.top<=dyBox/5               &&
	   dxRun/5 <= dxBox                     &&
	   dyRun <= THREE_HALF(dyBox)           &&
	   dyBox >= nMinLen						&&
	   IsMonotonous( pTrace, iBottom, iTop, 0, 6))
	{
		return GEST_SCROLLUP;
	}  
	
	// GEST_SCROLLDN
	if( (gtCheck&GEST_SCROLLDN)             &&
	   dyBox!=0                             &&
	   dxBox<=dyBox/4                       &&
	   box.bottom-yLast<=dyBox/5            &&
	   yFirst-box.top<=dyBox/5              &&
	   dxRun/5 <= dxBox                     &&
	   dyRun <= THREE_HALF(dyBox)           &&
	   dyBox >= nMinLen                     &&
	   IsMonotonous( pTrace, iTop, iBottom, 0, 6))
	{
		return GEST_SCROLLDN;
	}  
	
	// GEST_UNDO
	if( (gtCheck&GEST_UNDO)                 &&
	   dyBox!=0                             &&
	   dxBox*4<=dyBox                       &&
	   (box.bottom-yFirst)*4<=dyBox         &&
	   (box.bottom-yLast)*4<=dyBox          &&
	   dxRun/6 <= dxBox                     &&
	   dyRun/3 <= dyBox )
	{
        //find maximum from begin
        y1 = box.top;
        iFirstExtr = iTop; //CHE: was unassigned
        for(i=iFirst; i<iTop; i++)
		{
			if(pTrace[i].y>y1)
			{
				y1 = pTrace[i].y;
				iFirstExtr = i;
			}
		}
        if((box.bottom-y1)*6 > dyBox*5)
			goto bypass_undo;
        y2 = box.top;
        iLastExtr = iTop;
        for(i=iTop+1; i<=iLast; i++)
		{
			if(pTrace[i].y>y2)
			{
				y2 = pTrace[i].y;
				iLastExtr = i;
			}
		}
        if((box.bottom-y2)*6 > dyBox*5)
			goto bypass_undo;
		
        if(!IsMonotonous( pTrace, iFirstExtr, iTop, 0, 6))
			goto bypass_undo;
        if(!IsMonotonous( pTrace, iTop, iLastExtr, 0, 6))
			goto bypass_undo;
        return GEST_UNDO; 
	}  
bypass_undo:
	// GEST_SPELL
	if( (gtCheck&GEST_SPELL)                &&
	   dyBox!=0                             &&
	   dxBox<=dyBox/4                       &&
	   (yLast-box.top)*4<=dyBox             &&
	   (yFirst-box.top)*4<=dyBox            &&
	   dxRun/6 <= dxBox                     &&
	   dyRun/3 <= dyBox )
	{
		
        //find minimum from begin
        y1 = box.bottom;
        iFirstExtr = iBottom; 
        for(i=iFirst; i<iBottom; i++)
		{
			if(pTrace[i].y<y1)
			{
				y1 = pTrace[i].y;
				iFirstExtr = i;
			}
		}
        if((y1-box.top)*6 > dyBox*5)
			goto bypass_spell;
        y2 = box.bottom;
        iLastExtr = iBottom; 
        for(i=iBottom+1; i<=iLast; i++)
		{
			if(pTrace[i].y<y2)
			{
				y2 = pTrace[i].y;
				iLastExtr = i;
			}
		}
        if((y2-box.top)*6 > dyBox*5)
			goto bypass_spell;
		
        if(!IsMonotonous( pTrace, iFirstExtr, iBottom, 0, 6))
			goto bypass_spell;
        if(!IsMonotonous( pTrace, iBottom, iLastExtr, 0, 6))
			goto bypass_spell;
        return GEST_SPELL;
	}  
	
bypass_spell:
	int  iFar = GetFarFromChordIndex ( pTrace, iFirst, iLast);
	RECT rcBeg, rcEnd;
	rcBeg.top = rcBeg.bottom = yFirst;
	rcBeg.left = rcBeg.right = xFirst; 
	
	for(i=iFirst; i<=iFar; i++)
	{
		if(pTrace[i].x<rcBeg.left)
            rcBeg.left = pTrace[i].x;
		else if(pTrace[i].x>rcBeg.right)
			rcBeg.right = pTrace[i].x;
		if(pTrace[i].y<rcBeg.top)
            rcBeg.top = pTrace[i].y;
		else if(pTrace[i].y>rcBeg.bottom)
			rcBeg.bottom = pTrace[i].y;
	}
	
	rcEnd.top  = rcEnd.bottom = pTrace[iFar].y;
	rcEnd.left = rcEnd.right = pTrace[iFar].x; 
	for(i=iFar; i<=iLast; i++)
	{
		if(pTrace[i].x<rcEnd.left)
            rcEnd.left = pTrace[i].x;
		else if(pTrace[i].x>rcEnd.right)
			rcEnd.right = pTrace[i].x;
		if(pTrace[i].y<rcEnd.top)
            rcEnd.top = pTrace[i].y;
		else if(pTrace[i].y>rcEnd.bottom)
			rcEnd.bottom = pTrace[i].y;
	}
	float dxBeg = rcBeg.right-rcBeg.left;
	float dyBeg = rcBeg.bottom-rcBeg.top;
	float dxEnd = rcEnd.right-rcEnd.left;
	float dyEnd = rcEnd.bottom-rcEnd.top;
	
	// GEST_RETURN && GEST_SPACE && GEST_TAB && GEST_MENU
	if(
	   (gtCheck&(GEST_RETURN|GEST_SPACE|GEST_TAB|GEST_MENU))    &&
	   dyBox!=0                                 &&
	   dxBeg*3<dyBeg                            &&
	   dxEnd>dyEnd*4                            &&
	   IsMonotonous(pTrace, iFirst, iFar, 0, 6) &&
	   IsMonotonous(pTrace, iFar, iLast, 8, 0)  &&
	   //     dxBox/6<dyBox                     &&
	   dxEnd>dyBeg                              &&
	   dxRun/4 < dxBox                          &&
	   dyRun/4 < dyBox  )
	{
        if(pTrace[iFar].x>xLast)   
		{
            if(gtCheck&GEST_RETURN)
			{
				if  ( pTrace[iFar].y > yFirst )  //CHE
					return GEST_RETURN;
			}
            if(gtCheck&GEST_MENU)
			{
				if  ( pTrace[iFar].y < yFirst )
					return GEST_MENU;
			}
		}
        else
		{  
			if(gtCheck&GEST_SPACE)
			{
				if  ( dyBeg <= (dxEnd*3)/4 && pTrace[iFar].y > yFirst )
					return GEST_SPACE;
			}
			if(gtCheck&GEST_TAB)
			{
				if  ( pTrace[iFar].y < yFirst )
					return GEST_TAB;
			}
		}
	}  
	
	// GEST_DELETE
	if( (gtCheck&GEST_DELETE)           &&
	   dxBox!=0 && dyBox<=dxBox/3       &&
	   xFirst-box.left<=dxBox/5         &&
	   box.right-xLast<=dxBox/5         &&
	   IsMonotonous( pTrace, iLeft, iRight, 8, 0) &&
	   dxRun < THREE_HALF(dxBox)        &&
	   (dyRun-dyBox)/3 <= dyBox         &&
	   dxBox > nMinLen )
	{
		return GEST_DELETE;
	}  
	
	
	
	// GEST_BACK 
	if( (gtCheck&GEST_BACK)             &&
	   dxBox!=0 && dyBox<=dxBox/3       &&
	   xLast-box.left<=dxBox/5          &&
	   box.right-xFirst<=dxBox/5        &&
	   dxRun < THREE_HALF(dxBox)        &&
	   (dyRun-dyBox)/3 <= dyBox         &&
	   IsMonotonous( pTrace, iRight, iLeft, 8, 0) )
	{
		if ( dxBox > nMinLen )
			return GEST_BACK_LONG;
		return GEST_BACK;
	}  
	
	// GEST_SELECTALL
	if( (gtCheck & GEST_SELECTALL)              &&
	   dyBox != 0                               &&
	   dxBox != 0                               &&
	   (box.bottom-yFirst)*4 <= dyBox           &&
	   (xFirst-box.left)*4 <= dxBox             &&
	   (box.bottom-yLast)*4 <= dyBox            &&
	   (xLast-box.left)*4 <= dxBox              &&
	   (pTrace[iFar].y-box.top)*4 <= dyBox      &&
	   (box.right-pTrace[iFar].x)*4 <= dxBox    &&
	   IsMonotonous(pTrace, iFirst, iFar, 6, 8) &&
	   IsMonotonous(pTrace, iFar, iLast, 6, 8) )
	{
        return GEST_SELECTALL;
	}
	
	// GEST_CORRECT
	if (  (gtCheck&GEST_CORRECT)                &&
	   dyBox!=0                                 &&
	   pTrace[iFar].y - yFirst>0                &&
	   pTrace[iFar].y - yLast>0                 &&
	   xLast - pTrace[iFar].x>0                 &&
	   dyBeg <= (dyEnd*3)/4                     &&
	   dyEnd >= (dyBox*4)/5                     &&
	   dxBeg < 2*dyBeg                          &&
	   dxEnd < 2*dyEnd                          &&
	   dxEnd > dxBox/4                          &&
	   dxBeg < (dxBox*2)/3                      &&
	   pTrace[iFar].x - xFirst>-dyBeg           &&
	   IsMonotonous(pTrace, iFirst, iFar, 0, 8) &&
	   IsMonotonous(pTrace, iFar, iLast, 6, 8)  )
	{
		float	CtgBegMin = -113, CtgBegMax = 15;
		float	CtgEndMin = 31, CtgEndMax = 113;
		float	iCtgBeg, iCtgEnd;
		if((pTrace[iFar].x - xFirst)>0 && dyEnd>dyBeg*2)
		{
            CtgBegMin = -161;
            CtgBegMax = 17;
            CtgEndMin = 20;
            CtgEndMax = 133;
		}
		
		iCtgBeg = (xFirst-pTrace[iFar].x)*100/dyBeg;
		iCtgEnd = dxEnd*100/dyEnd;
		if(iCtgBeg>=CtgBegMin && iCtgBeg<=CtgBegMax && iCtgEnd>=CtgEndMin && iCtgEnd<=CtgEndMax)
		{
			return GEST_CORRECT;
		}
	}  
	// GEST_COPY
	if(
	   (gtCheck&GEST_COPY)                  &&
	   dxBox != 0                           &&
	   dyBox <= dxBox/4                     &&
	   (xLast-box.left)*5<=dxBox            &&
	   (xFirst-box.left)*5<=dxBox           &&
	   dxRun/3 <= dxBox                     &&
	   dyRun/4 <= dyBox )
	{
		
        //find x-minimum from begin
        x1 = xFirst;
        iFirstExtr = iFirst;
        for(i=iFirst; i<iRight; i++)
		{
			if(pTrace[i].x<x1)
			{
				x1 = pTrace[i].x;
				iFirstExtr = i;
			}
		}
		/*
        if((x1-box.left)*6 > dxBox*5) //???
			goto bypass_copy;
		*/
        x2 = box.right;
        iLastExtr = iRight; 
        for(i=iRight+1; i<=iLast; i++)
		{
			if(pTrace[i].x<x2)
			{
				x2 = pTrace[i].x;
				iLastExtr = i;
			}
		}
		/*
        if((x2-box.left)*6 > dxBox*5)
			goto bypass_copy;
		*/
		
        if(!IsMonotonous( pTrace, iFirstExtr, iRight, 6, 0))
			goto bypass_copy;
        if(!IsMonotonous( pTrace, iRight, iLastExtr, 6, 0))
			goto bypass_copy;
        return GEST_COPY;
	}  
	
bypass_copy:
	// GEST_CUT
	if(
	   (gtCheck&GEST_CUT)                   &&
	   dxBox != 0                           &&
	   dyBox <= dxBox/4                     &&
	   (box.right-xLast)*5 <= dxBox         &&
	   (box.right-xFirst)*5 <= dxBox        &&
	   dxRun/3 <= dxBox                     &&
	   dyRun/4 <= dyBox
	   )
	{
		
        //find x-maximum from begin
        x1 = xFirst;
        iFirstExtr = iFirst; //CHE: was unassigned
        for( i = iFirst; i < iLeft; i++ )
		{
			if(pTrace[i].x>x1)
			{
				x1 = pTrace[i].x;
				iFirstExtr = i;
			}
		}
		/*
        if((box.right-x1)*6 > dxBox*5)
			goto bypass_cut;
		*/
        x2 = box.left;
        iLastExtr = iLeft; 
        for( i = iLeft+1; i <= iLast; i++ )
		{
			if ( pTrace[i].x > x2 )
			{
				x2 = pTrace[i].x;
				iLastExtr = i;
			}
		}
		/*
        if((box.right-x2)*6 > dxBox*5)
			goto bypass_cut;
		*/
        if(!IsMonotonous( pTrace, iFirstExtr, iLeft, 6, 0))
			goto bypass_cut;
        if(!IsMonotonous( pTrace, iLeft, iLastExtr, 6, 0))
			goto bypass_cut;
        return GEST_CUT;
	}  
bypass_cut:
	
	// GEST_PASTE
	if(
	   (gtCheck&GEST_PASTE)                 &&
	   dyBox != 0                           &&
	   pTrace[iFar].y - yFirst<0            &&
	   pTrace[iFar].y - yLast<0             &&
	   xLast - pTrace[iFar].x>0             &&
	   pTrace[iFar].x - xFirst>0            &&
	   dxRun/3 <= dxBox                     &&
	   dyRun/4 <= dyBox                     &&
	   dxBeg <= dxEnd*2                     &&
	   dxEnd <= dxBeg*2                     &&
	   dyBeg <= (dyEnd*3)/2                 &&
	   dyEnd <= (dyBeg*3)/2                 &&

	   ((dyBeg >= (dyBox*4)/5 && dyEnd >= (dyBox*2)/3) ||
        (dyEnd >= (dyBox*4)/5 && dyBeg >= (dyBox*2)/3))
                                            &&
	   dxBeg <= (dyBeg*3)/2                 &&
	   dyBeg <= dxBeg*3                     &&
	   dxEnd <= (dyEnd*3)/2                 &&
	   dyEnd <= dxEnd*3                     &&
	   
	   IsMonotonous(pTrace, iFirst, iFar, 6, 8) &&
	   IsMonotonous(pTrace, iFar, iLast, 6, 8) )
	{
		return GEST_PASTE;
	}  
	
	// GEST_LOOP
	if( (gtCheck&GEST_LOOP) && dxBox!=0  && dyBox!=0 )
	{
		float lSquare, lBoxSquare = dxBox*dyBox;
		if ( GestClosedSquare( &pTrace[iFirst], iLast-iFirst-1, &lSquare) )
		{
			if ( lSquare < 0 )
				lSquare = -lSquare;
			if ( 8.0 * dxBox < 5.5 * dxRun && 8.0 * dyBox < 6.0 * dyRun && 3.0 * lSquare > lBoxSquare )
			{
				return GEST_LOOP;
			}
		}
	}  
	
	if ( (gtCheck&(GEST_LEFTARC)) && dxBox > 0 && dyBox >= dxBox/4 )
	{
		if ( pTrace[iLast].x > pTrace[iFirst].x && pTrace[iFar].y > pTrace[iLast].y && pTrace[iFar].y > pTrace[iFirst].y
			// && pTrace[iFar].x < pTrace[iLast].x && pTrace[iFar].x > pTrace[iFirst].x 
			)
		{
			return GEST_LEFTARC;
		}
		if ( pTrace[iLast].x < pTrace[iFirst].x && pTrace[iFar].y < pTrace[iLast].y && pTrace[iFar].y < pTrace[iFirst].y
			// && pTrace[iFar].x > pTrace[iLast].x && pTrace[iFar].x < pTrace[iFirst].x 
			)
		{
			return GEST_LEFTARC;
		}		
		/*
		 POINTS	ptc;
		 int		dx, dy, x3;
		 ptc.x = pTrace[iLast].x + (pTrace[iFirst].x - pTrace[iLast].x)/2;
		 ptc.y = pTrace[iLast].y + (pTrace[iFirst].y - pTrace[iLast].y)/2;
		 
		 // BOOL bMono = IsMonotonous( pTrace, iLast, iFirst, 8, 8 );
		 
		 dx = (pTrace[iFar].x - ptc.x);
		 dy = (pTrace[iFar].y - ptc.y);
		 x1 = _isqrt( dy * dy + dx * dx );
		 dx = (pTrace[iFirst].x - ptc.x);
		 dy = (pTrace[iFirst].y - ptc.y);
		 x2 = _isqrt( dy * dy + dx * dx );
		 dx = (pTrace[iLast].x - ptc.x);
		 dy = (pTrace[iLast].y - ptc.y);
		 x3 = _isqrt( dy * dy + dx * dx );
		 
		 if ( abs(x2-x1) < dyBox/5 && abs(x3-x1) < dyBox/5 )
		 return GEST_LEFTARC;
		 */
		
	}
	if ( (gtCheck&(GEST_RIGHTARC)) && dxBox > 0 && dyBox >= dxBox/4 )
	{
		if ( pTrace[iLast].x > pTrace[iFirst].x && pTrace[iFar].y < pTrace[iLast].y && pTrace[iFar].y < pTrace[iFirst].y
			// && pTrace[iFar].x < pTrace[iLast].x && pTrace[iFar].x > pTrace[iFirst].x 
			)
		{
			return GEST_RIGHTARC;
		}
		if ( pTrace[iLast].x < pTrace[iFirst].x && pTrace[iFar].y > pTrace[iLast].y && pTrace[iFar].y > pTrace[iFirst].y
			// && pTrace[iFar].x > pTrace[iLast].x && pTrace[iFar].x < pTrace[iFirst].x 
			)
		{
			return GEST_RIGHTARC;
		}
		
		/*
		 POINTS	ptc;
		 int		dx, dy, x3;
		 ptc.x = pTrace[iLast].x + (pTrace[iFirst].x - pTrace[iLast].x)/2;
		 ptc.y = pTrace[iLast].y + (pTrace[iFirst].y - pTrace[iLast].y)/2;
		 
		 
		 dx = (pTrace[iFar].x - ptc.x);
		 dy = (pTrace[iFar].y - ptc.y);
		 x1 = _isqrt( dy * dy + dx * dx );
		 dx = (pTrace[iFirst].x - ptc.x);
		 dy = (pTrace[iFirst].y - ptc.y);
		 x2 = _isqrt( dy * dy + dx * dx );
		 dx = (pTrace[iLast].x - ptc.x);
		 dy = (pTrace[iLast].y - ptc.y);
		 x3 = _isqrt( dy * dy + dx * dx );
		 
		 if ( abs(x2-x1) < dyBox/5 && abs(x3-x1) < dyBox/5 )
		 return GEST_RIGHTARC;
		 */
		
	}
	return GEST_NONE;
}

/*******************************************************************************************/
		
//from low_util ???
static _BOOL IsMonotonous( CGPoint * pTrace, int iLeft, int iRight, int iXth, int iYth)
{
	float iDX;
	float iDY;
	int kxIncrease, kyIncrease;
	int i;
	float xCurr, yCurr, dx, dy;
	if(iLeft>iRight)
	{
		i = iLeft;
		iLeft = iRight;
		iRight = i;
	}
	iDX = pTrace[iRight].x - pTrace[iLeft].x;
	iDY = pTrace[iRight].y - pTrace[iLeft].y;

	kxIncrease = (iDX>=0)?1:-1;
	kyIncrease = (iDY>=0)?1:-1;
	if(iDX<0)
		iDX = -iDX;
	if(iDY<0)
		iDY = -iDY;
	xCurr = pTrace[iLeft].x;
	yCurr = pTrace[iLeft].y;
	
	for(i=iLeft+1; i<=iRight; i++)
	{
		if((pTrace[i].x - xCurr)*kxIncrease>0)
		{
			xCurr = pTrace[i].x;
			dx = 0;
		}
		else
		{
            dx=(xCurr - pTrace[i].x)*kxIncrease;
		}
		if((pTrace[i].y - yCurr)*kyIncrease>0)
		{
			yCurr = pTrace[i].y;
			dy = 0;
		}
		else
		{
			dy=(yCurr - pTrace[i].y)*kyIncrease;
		}
		if(dx*iXth>iDX || dy*iYth>iDY)
            return false;
	}
	return true;
}

static int  GetFarFromChordIndex ( CGPoint *  pTrace, int iLeft, int iRight )
{
	int    i;
	int    iMostFar;
	float  dxRL, dyRL;
	float   ldConst;
	float  ldMostFar, ldCur;
	_BOOL   bIncrEqual;
	_BOOL   bFlatPlato;

      /*                                                  */
      /*                      O <-iRight                  */
      /*                     . O                          */
      /*                    .   O                         */
      /*                   .    O                         */
      /*                  .    O                          */
      /*                 .     O                          */
      /*                . ..   O                          */
      /*               .    .. O                          */
      /*          OOO .       .O                          */
      /*         O   O   O    OO <-iMostFar               */
      /*        O   . OOO O  O                            */
      /*        O  .       OO                             */
      /*       O  .                                       */
      /*       O .                                        */
      /*        O <-iLeft                                 */
      /*                                                  */
      /*   <Distance> ~ <dY> = y - yStraight(x) =         */
      /*                                                  */
      /*                                        x-xLeft   */
      /*       = y - yLeft - (yRight-yLeft) * ------------*/
      /*                                      xRight-xLeft*/
      /*                                                  */
      /*       ~ y*(xR-xL) - x*(yR-yL) +                  */
      /*          + xL*(yR-yL) - yL*(xR-xL)               */
      /*                                                  */
      /*    And no problems with zero divide!             */
      /*                                                  */

	dxRL = pTrace[iRight].x - pTrace[iLeft].x;
	dyRL = pTrace[iRight].y - pTrace[iLeft].y;
	ldConst = (pTrace[iLeft].x*dyRL - pTrace[iLeft].y*dxRL);
	bFlatPlato = true;
	bIncrEqual = false;
	
	for  ( i=iLeft+1, iMostFar=iLeft, ldMostFar=0L; i<=iRight; i++ ) 
	{
		if  ( pTrace[i].y == TRACE_BREAK )  
		{
			bFlatPlato = false;
			continue;
		}
		
		ldCur = (pTrace[i].y*dxRL - pTrace[i].x*dyRL) + ldConst;
		if(ldCur<0)
			ldCur = -ldCur;
		
		if  ( ldCur > ldMostFar )  
		{
			ldMostFar  = ldCur;
			iMostFar   = i;
			bIncrEqual = false;
			bFlatPlato = true;
		}
		else if ( bFlatPlato  &&  (ldCur == ldMostFar) ) 
		{
			if  ( bIncrEqual )
				iMostFar++;
			bIncrEqual = !bIncrEqual;
		}
		else
			bFlatPlato = false;
		
	}
	
	return  iMostFar;

} /*GetFarFromChordIndex*/


/********************************************************************/

/*   The following function calculates the square within
 * the trajectory part.  Clockwise path gives positive square,
 * counterclockwise - negative.
 *   At the following example, if "A" is the starting point
 * and "B" - ending one, then the "1" area will give
 * negative square, "2" area - positive square:
 *
 *                             ooo<ooo
 *                       oo<ooo       ooooo
 *                      o                  oo
 *                     o                     oo
 *                   oo          1             o
 *                  o                          o
 *                 o                        ooo
 *                 o                    oooo
 *   A ooooo>oooooooooooo>ooooooo>oooooo
 *      |        oo
 *       |  2   o
 *        |    o
 *         ooo
 *        B
 */

static _BOOL  GestClosedSquare( CGPoint * pTrace, int cTrace, float *plSquare)
{
	int    i, ip1;
	float   lSum;

	if  ( plSquare==NULL || pTrace==NULL || pTrace[0].y==TRACE_BREAK)
		return  false;
	while(cTrace>0 && pTrace[cTrace-1].y==TRACE_BREAK)
	{
		cTrace--;
	}
	if(cTrace<=2)
		return false;

	/*  Regular case, count integral: */
	/*  First, get the square under the chord connecting */
	/* the ends and pre-subtract it from the sum:        */
	lSum = ((pTrace[cTrace-1].y+pTrace[0].y) * (pTrace[cTrace-1].x-pTrace[0].x));

	/*  Then count the square under the trajectory: */
	for  ( i=0, ip1=i+1; i<cTrace-1; i++, ip1++ )  
	{
		if ( pTrace[ip1].y == TRACE_BREAK )
			return false;
		lSum -= ((pTrace[i].y+pTrace[ip1].y) * (pTrace[ip1].x-pTrace[i].x));
	}

	*plSquare =   (lSum/2);
	return true;
} /*ClosedSquare*/

