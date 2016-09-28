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

#ifndef LSTRIP

/*****************************************************************************/
/*****  E R I C  *************************************************************/
/*****************************************************************************/

#include "hwr_sys.h"
#include "ams_mg.h"
#include "lowlevel.h"
#include "lk_code.h"
#include "def.h"
#include "calcmacr.h"
#include "langid.h"
#include "reco.h"

#ifdef  FORMULA
#include "frm_con.h"
#endif /*FORMULA*/

/*****************************************************************************/
/***** COMMON CODE ***********************************************************/
/*****************************************************************************/

static _BOOL IsUpperSpl( p_SPECL pElem )
{
    if ( IsUpperElem( pElem ) )
    {
        if ( pElem->mark == END && HEIGHT_OF(pElem) > _UI2_ )
            return _FALSE;
        else
            return _TRUE;
    }
    if ( pElem->code == _Gr_ || pElem->code == _Gl_ )
    {
        if ( HEIGHT_OF(pElem) < _MD_ )
            return _TRUE;
    }

    return _FALSE;
}

#define  STR_SIZE (STR_DOWN-STR_UP)

static p_SPECL FindApsPlace(_INT lang, low_type _PTR low_data,p_SPECL pPt1, _BOOL bMove, p_SHORT bAfter );
static p_SPECL GetNextUpElem( p_SPECL pSpcl );
static p_SPECL GetPrevUpElem( p_SPECL pSpcl );
static _VOID AdjustBegEnd( p_SPECL pPoint );

_VOID AdjustBegEnd( p_SPECL pPoint )
{
    p_SPECL pPrev, pNext;

    if ( (pPrev = pPoint->prev) == _NULL || (pNext = pPoint->next) == _NULL )
    {
        return;
    }
    if ( IsAnyBreak( pPrev ) )
    {
        if ( pPrev->iend == pPoint->ibeg )
            pPrev->iend = pNext->ibeg;
    }
    if ( IsAnyBreak( pNext ) )
    {
        if ( pNext->ibeg == pPoint->iend )
            pNext->ibeg = pPrev->iend;
    }
}

p_SPECL GetNextUpElem( p_SPECL pSpcl )
{

    while ( pSpcl->next != _NULL )
    {
        pSpcl = pSpcl->next;
        if ( IsUpperSpl( pSpcl ) )
            return (pSpcl);
    }
    return (_NULL);
}

p_SPECL GetPrevUpElem( p_SPECL pSpcl )
{
    while ( pSpcl != _NULL )
    {
        if ( IsUpperSpl( pSpcl ) )
            return (pSpcl);
        pSpcl = pSpcl->prev;
    }
    return (_NULL);
}

/*****************************************************************************/
/***** FOR FRENCH ************************************************************/
/*****************************************************************************/

#define MIN_SIZE_REG 450
#define MIN_SIZE_SEP 300
#define MIN_SIZE_BEG 150
#define MIN_SIZE_END  15
#define MIN_HOLE_APS  15

#define SANG 1
#define SNLW 2
#define SNUP 4

static _BOOL ProcessStatistic(_INT xHole,_INT xSize,_INT hL,_INT hR,_INT hA,_INT sA,_BOOL bSep);
static _BOOL IsNearE(p_SPECL, p_SHORT x, p_SHORT y,_BOOL bL);
static _BOOL IsBreak(p_SPECL pElem);
static _BOOL SomeLetters(p_SPECL pBack,p_SHORT x,p_SHORT y);
static p_SPECL Spl(p_SPECL pSpl,_INT i,int nMode);

_BOOL RestoreApostrophFrench(p_low_type low_data, p_SPECL pCurr)
{
    _INT lang = low_data->rc->lang;
    _INT hL,hR=0,hA,sA;
    _BOOL bSep;
    _SHORT bAfter;
    _RECT rc;
    _INT iSafe,xPos,yPos;
    _INT xHole,xSize;
    p_SPECL pPos;
    p_SPECL next;
    p_SPECL prev;
    p_SHORT x;
    p_SHORT y;

    x = low_data->x;
    y = low_data->y;

    bSep = (low_data->rc->low_mode & LMOD_SEPARATE_LET)!=0;

    if(pCurr->code!=_ST_)
        return(_FALSE);
    
    /* height */
    xPos = x[MID_POINT(pCurr)];
    yPos = y[MID_POINT(pCurr)];
    if(yPos>STR_UP+ONE_THIRD(STR_SIZE))
    {
        return(_FALSE);
    }
    /* vert or horiz */
    GetTraceBox(x,y,pCurr->ibeg,pCurr->iend,(p_RECT)&rc);

    if( 2*(rc.bottom-rc.top) < (rc.right-rc.left) )
    {
        return _FALSE;
    }

    /* find place */
    pPos = FindApsPlace(lang, low_data, pCurr,_FALSE,&bAfter);

    if(pPos==_NULL)
    {
        return(_FALSE);
    }
    /* look around: are upper elements near? */
    next = GetNextUpElem(pPos);
    prev = GetPrevUpElem(pPos);

    iSafe = -10;
    while(next!=_NULL && prev!=_NULL && x[MID_POINT(prev)]>x[MID_POINT(next)])
    {
        _INT dxPrev = x[MID_POINT(prev)] - xPos;
        _INT dxNext = x[MID_POINT(next)] - xPos;
        _INT dx;

        TO_ABS_VALUE(dxPrev);
        TO_ABS_VALUE(dxNext);

        if( dxPrev < dxNext )
        {
            //next = prev;
            dx = x[MID_POINT(prev)] - xPos;
        }
        else
        {
            //prev = next;
            dx = x[MID_POINT(next)] - xPos;
        }
        if( dx < 0 ) next = GetNextUpElem(next);
        else prev = GetPrevUpElem(prev->prev);

        if(++iSafe == 0)
        {
            break;
        }
    }
    if(next==_NULL && prev==_NULL)
    {
        return(_FALSE);
    }
    /* only one letter is possible before apostrophe */
    if( !SomeLetters(pPos,x,y) )
        return _FALSE;

    /* on the end */
    if(prev!=_NULL && next==_NULL)
    {
        _RECT rc1;
        _RECT rc;

        GetTraceBox(x,y,prev ->ibeg,prev ->iend,(p_RECT)&rc1);
        GetTraceBox(x,y,pCurr->ibeg,pCurr->iend,(p_RECT)&rc );

        xSize = (rc.right-rc.left)*(rc.right-rc.left)+(rc.bottom-rc.top)*(rc.bottom-rc.top);

        if(xSize>MIN_SIZE_END)
        {
        }
        else
        {
            return _FALSE;
        }
    }
    /* Is near e? */
    if(IsNearE(pPos,x,y,_FALSE) && next)
    {
        return _FALSE;
    }
    /* metric parameters: */
    {
        _RECT rc1;
        _RECT rc2;
        p_SPECL pPrev=_NULL;
        p_SPECL pNext=_NULL;

        GetTraceBox(x,y,pCurr->ibeg,pCurr->iend,(p_RECT)&rc);
        xSize = (rc.right-rc.left)*(rc.right-rc.left)+(rc.bottom-rc.top)*(rc.bottom-rc.top);
        {
            xHole = STR_SIZE;

            if(prev) pPrev = prev->next;
            if(next) pNext = next->prev;

            if(pPrev && HEIGHT_OF(pPrev)> _MD_)
            {
                pPrev = pPrev->prev;
            }
            if(pPrev && (pPrev->code==_ST_ || pPrev==pPos || IsAnyBreak(pPrev)))
            {
                pPrev = pPrev->prev;
            }
            if(pNext && (pNext->code==_ST_ || pNext==pPos || IsAnyBreak(pNext)))
            {
                pNext = pNext->next;
            }
            if(pPrev || pNext)
            {
                if(pPrev)
                {
                    GetTraceBox(x,y,0,pPrev->iend,(p_RECT)&rc1);
                }
                if(pNext)
                {
                    _SHORT iend = next->iend;

                    if(next->next && next->next->code!=_ST_) iend = next->next->iend;
                    GetTraceBox(x,y,pNext->ibeg, iend,(p_RECT)&rc2);
                }
                if(pPrev && pNext) xHole = rc2.left-rc1.right;

                if((pPrev && rc1.right>=rc.left) || (pNext && rc2.left<=rc.right))
                {
                    return _FALSE;
                }
            }
        }
        if(prev!=_NULL)
        {
            hA = rc.top;
            sA = rc.bottom-rc.top;

            GetTraceBox(x,y,0,prev->iend,(p_RECT)&rc);
            hL = rc.top;
        }
        else
        {
            hA = hL = sA = 0;
        }
        if(next==_NULL && xSize<=MIN_SIZE_END )
        {
            return _FALSE;
        }
        if(prev==_NULL && xSize<=MIN_SIZE_BEG )
        {
            return _FALSE;
        }
        if(prev!=_NULL && next!=_NULL)
        {
            if(!ProcessStatistic(xHole,xSize,hL,hR,hA,sA,bSep)) return _FALSE;
        }
    }

    /* OK! It's apostrophe! */
    if(prev!=_NULL && next==_NULL)
    {
        IsNearE(pPos,x,y,_TRUE);
    }
    /* Move to nearest element: */
    FindApsPlace(lang, low_data, pCurr,_TRUE,&bAfter);

    return(_TRUE);
}

static _BOOL IsLowerSpl(p_SPECL pElem)
{
    if(IsLowerElem(pElem))
    {
        if(pElem->mark==END && HEIGHT_OF(pElem)<_UI2_) return _FALSE;
        else return _TRUE;
    }
    return _FALSE;
}

p_SPECL Spl(p_SPECL pSpl,_INT i,int nMode)
{
    int iDir = i>0?-1:1;

    while(pSpl!=_NULL)
    {
        if(i==0) break;

        if(iDir>0) pSpl = pSpl->prev;
        if(iDir<0) pSpl = pSpl->next;
        if(pSpl==_NULL) break;

        if((nMode & SANG) && IsAnyAngle(pSpl)) continue;
        if((nMode & SNUP) && !IsUpperSpl(pSpl)) continue;
        if((nMode & SNLW) && !IsLowerSpl(pSpl)) continue;
        if(IsAnyBreak(pSpl)) continue;
        if(pSpl->code==_ST_) continue;
        if(pSpl->code==_XT_) continue;
        if(pSpl->prev==_NULL) continue;

        i+=iDir;
    }
    return pSpl;
}

_BOOL SomeLetters(p_SPECL pBack, p_SHORT x, p_SHORT y)
{
    _SHORT nUp=0,nLw=0;
    _SHORT hUp=0,hLw=0;
    _SHORT nMM=0;
    _BOOL bMM=_FALSE;
    _SHORT nDn=0;
    _SHORT nTp=0;
    _BOOL bAA = _FALSE;
    _BOOL bNN = _FALSE;

    if( Spl(pBack,-2,SNUP | SANG)!=_NULL && Spl(pBack,-2,SNLW | SANG)!=_NULL )
    {
        p_SPECL pUp1 = Spl(pBack,-2,SNUP | SANG);
        p_SPECL pUp2 = Spl(pBack,-1,SNUP | SANG);
        p_SPECL pLw1 = Spl(pBack,-2,SNLW | SANG);
        p_SPECL pLw2 = Spl(pBack,-1,SNLW | SANG);

        bAA=( (pLw1->code==_UDC_ || (pLw1->code==_UD_ && pUp1->code==_UUR_ && pUp2->code!=_UU_))
                && (HEIGHT_OF(pLw1) >=_DI1_ && HEIGHT_OF(pLw1)<=_DE1_)
                && (HEIGHT_OF(pLw2) >=_DI1_ && HEIGHT_OF(pLw2)<=_DE1_)
                && (HEIGHT_OF(pUp1) >=_UE2_ && HEIGHT_OF(pUp1)<=_UI2_)
                && (HEIGHT_OF(pUp2) >=_UE2_ && HEIGHT_OF(pUp2)<=_UI2_)
                && (HEIGHT_OF(pUp1) <= HEIGHT_OF(pUp2) )
                && (HEIGHT_OF(pLw1) >= HEIGHT_OF(pLw2) ) );
        if(bAA)
        {
            return _FALSE;
        }
    }
    if( Spl(pBack,-3,SNUP | SANG)!=_NULL && Spl(pBack,-2,SNLW | SANG)!=_NULL )
    {
        p_SPECL pUp1 = Spl(pBack,-3,SNUP | SANG);
        p_SPECL pUp2 = Spl(pBack,-2,SNUP | SANG);
        p_SPECL pUp3 = Spl(pBack,-1,SNUP | SANG);
        p_SPECL pLw1 = Spl(pBack,-2,SNLW | SANG);
        p_SPECL pLw2 = Spl(pBack,-1,SNLW | SANG);

        bNN=( (pUp2->code==_IU_ || pUp2->code==_UU_)
                && (pUp1->mark==BEG && pUp3->mark==END )
                && (HEIGHT_OF(pLw1)>=_DI1_ && HEIGHT_OF(pLw1)<=_DE1_)
                && (HEIGHT_OF(pLw2)>=_DI1_ && HEIGHT_OF(pLw2)<=_DE1_)
                && (HEIGHT_OF(pUp1)<=_UI2_)
                && (HEIGHT_OF(pUp2)<=_UI2_)
                && (HEIGHT_OF(pUp3)<=_UI2_)
                && ( 4*(x[MID_POINT(pUp2)]-x[MID_POINT(pUp1)]) <= (x[MID_POINT(pUp3)]-x[MID_POINT(pUp2)]) ));
    }
    for(;pBack!=_NULL;pBack=pBack->prev)
    {
        _BOOL bUp = IsUpperSpl(pBack);
        _BOOL bLw = IsLowerSpl(pBack);

        if(bUp)
        {
            nUp++;
            if(HEIGHT_OF(pBack)<=_UE1_) nTp++;
        }
        else if(bLw)
        {
            nLw++;
            if(HEIGHT_OF(pBack)>=_DE2_) nDn++;
        }
        else
        {
            //if(nUp+nLw>0 && ( IsBreak(pBack) )) goto NO;
            continue;
        }
        if(nMM>=0) nMM++;

        if(nMM==1)
        {
            hLw = HEIGHT_OF(pBack);
            if(!bLw || hLw<_DI1_ || hLw>_DE1_) nMM=-1;
        }
        else if(nMM==2 || nMM==4)
        {
            if(nMM==2)
            {
                hUp = HEIGHT_OF(pBack);
                if(hUp<_UE2_ || hUp>_UI2_) nMM=-1;
            }
            if(!bUp || (pBack->code!=_IU_ && pBack->code!=_UU_) || hUp != HEIGHT_OF(pBack))
                nMM=-1;
        }
        else if(nMM==3 || nMM==5)
        {
            if(!bLw || (pBack->code!=_ID_ && pBack->code!=_UD_) || hLw != HEIGHT_OF(pBack)) 
                nMM=-1;
        }
        else if(nMM==6)
        {
            bMM = bUp && hUp == HEIGHT_OF(pBack);
        }

        if(nUp>4)
            goto NOWORK;
        if(nUp>3 && nDn==1)
            continue;
        if(nUp>3)
            goto NOWORK;
        if(nUp>2 && nLw==3 && bMM)
            continue;
        if(nUp>2 && nDn==1 && !nTp)
            continue;
        if(nUp>2 && nLw==2 && bNN)
            continue;
        if(nUp>2)
            goto NOWORK;
    }
    if(pBack==_NULL)
    {
        return _TRUE;
    }
NOWORK:

    return _FALSE;
}

_BOOL ProcessStatistic(_INT xHole,_INT xSize,_INT hL,_INT hR,_INT hA,_INT sA,_BOOL bSep)
{
    if(xHole<MIN_HOLE_APS )
    {
        goto NOWORK;
    }
    if(bSep)
    {
        if(xSize<MIN_SIZE_SEP)
        {
            goto NOWORK;
        }
    }
    else
    {
        if(xSize<MIN_SIZE_REG)
        {
            goto NOWORK;
        }
    }
    if(hL<hA-sA )
    {
        goto NOWORK;
    }
    goto OKWORK;
NOWORK:
    return _FALSE;
OKWORK:
    return _TRUE;
}

/*****************************************************************************/
_BOOL IsBreak(p_SPECL pElem)
{
    if(REF(pElem)->code==_ZZZ_) return _TRUE;
    //    if(REF(pElem)->code==_ZZ_ ) return _TRUE;
    //    if(REF(pElem)->code==_Z_  ) return _TRUE;
    if(REF(pElem)->code==_FF_ ) return _TRUE;

    return _FALSE;
}
/*****************************************************************************/

/*  Returns _TRUE, if the place was found; _FALSE otherwise. */

p_SPECL FindApsPlaceFrench(low_type _PTR low_data,p_SPECL pPoint,_BOOL bMove,p_SHORT bAfter)
{
    p_SHORT x = low_data->x;
    p_SHORT y = low_data->y;
    p_SPECL pCur;
    p_SPECL pBest;
    _INT dxBest, dxCur, xCur, xColon;

    _BOOL bFind;
    _BOOL bShft = 0;
    _BOOL bCurr;
    p_SPECL pNewBrk;

    /*  Find the best place to put apostroph: */

    xColon = x[MID_POINT(pPoint)];

    /* Scan upper elements to find the best apostroph place:  */

    for (   pCur=low_data->specl->next, dxBest=ALEF, pBest=_NULL;
            pCur!=_NULL;
            pCur=pCur->next )
    {
        if( pCur!=pPoint && IsUpperSpl(pCur) )
        {
            if( pCur->prev == _NULL )
            {
                if( pCur->next == _NULL || y[pCur->iend] == BREAK )
                {
                    return _NULL;
                }
                xCur = x[pCur->iend];
            }
            else if( pCur->next == _NULL )
            {
                if( y[pCur->ibeg] == BREAK )
                {
                    return _NULL;
                }
                xCur = x[pCur->ibeg];
            }
            else
            {
                if( y[pCur->ibeg] == BREAK || y[pCur->iend] == BREAK )
                {
                    return _NULL;
                }
                if( IsBreak(pCur) )
                {
                    if(x[pCur->ibeg]<x[pCur->iend]) xCur = x[MID_POINT(pCur)];
                    else xCur = x[pCur->ibeg];
                }
                else xCur = x[MID_POINT(pCur)];
            }
            dxCur = xCur - xColon;
            bCurr = dxCur>0;
            TO_ABS_VALUE(dxCur);

            if( dxCur < dxBest )
            {
                pBest = pCur;
                dxBest = dxCur;
                bShft = bCurr;
            }
        }
    }
    /*  If there is no place to put apostrophe, just exit: */
    if(pBest == _NULL) return _NULL;

    if(bShft && pBest->prev!=_NULL) pBest=pBest->prev;

    bFind = _FALSE;
    *bAfter = _TRUE;
    while(pBest)
    {
        if(IsUpperSpl(pBest))
        {
            if(x[MID_POINT(pBest)]<xColon) break;
        }
        if(pBest->prev!=_NULL) pBest=pBest->prev;
        if(pBest==low_data->specl || IsBreak(pBest))
        {
            bFind =_TRUE;
            *bAfter =_FALSE;
            break;
        }
    }
    if(!bFind) while(pBest->next)
    {
        if(IsUpperSpl(pBest->next))
        {
            if(x[MID_POINT(pBest->next)]>xColon) break;
        }
        if(IsBreak(pBest->next))
        {
            *bAfter =_FALSE;
            pBest=pBest->next;
            break;
        }
        pBest=pBest->next;
    }
    if ( pBest!=low_data->specl && !IsBreak(pBest) ) return(_FALSE);

    if(!bMove) return(pBest);

    if ( pBest!=low_data->specl && !IsAnyBreak(pBest) )
    {
        pNewBrk = NewSPECLElem( low_data );
        if(pNewBrk == _NULL) return _NULL;
        Insert2ndAfter1st(pBest, pNewBrk );
        pNewBrk->ibeg =
        pNewBrk->iend = pBest->iend;
        pBest = pNewBrk;
    }
    pBest->code = _FF_;
    pBest->mark = DROP;
    pBest->other = NO_PENALTY;
    ASSIGN_HEIGHT(pBest,_MD_);

    AdjustBegEnd( pPoint );

    Move2ndAfter1st( pBest, pPoint );
    if(pBest->ibeg < pPoint->ibeg) pBest->iend = pPoint->ibeg;

    if( pPoint->next != _NULL )
    {
        if( !IsAnyBreak(pPoint->next) )
        {
            pNewBrk = NewSPECLElem( low_data );
            if(pNewBrk == _NULL) return _FALSE;
            Insert2ndAfter1st( pPoint, pNewBrk );
        }
        pNewBrk = pPoint->next;

        pNewBrk->code = _FF_;
        pNewBrk->mark = DROP;
        pNewBrk->other = NO_PENALTY;
        ASSIGN_HEIGHT(pNewBrk,_MD_);

        pNewBrk->ibeg =
        pNewBrk->iend = pPoint->iend;
    }
    return pBest;
}

/*****************************************************************************/

_BOOL IsNearE(p_SPECL pPos, p_SHORT x, p_SHORT y,_BOOL bL)
{
    p_SPECL pPos1 = pPos->prev;
    p_SPECL pPos2;
    p_SPECL pPos3;

    if(!pPos1) return _FALSE;
    if( pPos1->code==_ST_ ) pPos1 = pPos1->prev;
    if(!pPos1) return _FALSE;

    if(bL)
    {
        for(pPos2=GetPrevUpElem(pPos1->prev);;pPos2=GetPrevUpElem(pPos2->prev))
        {
            if(pPos2==_NULL) return _FALSE;
            if(HEIGHT_OF(pPos2)< _MD_) break;
        }
        for(pPos3=GetPrevUpElem(pPos2->prev);;pPos3=GetPrevUpElem(pPos3->prev))
        {
            if(pPos3==_NULL) break;
            if(HEIGHT_OF(pPos3)< _MD_) return _FALSE;
        }
        ASSIGN_HEIGHT(pPos2,_UE1_);
        goto OK;
    }
    if( pPos1->code==_UDR_ || (pPos1->code==_ID_ && pPos1->mark==END) )
    {
        _SHORT h;

        pPos2 = pPos1->prev;
        if(!pPos2) return _FALSE;

        if(IsAnyAngle(pPos2) && HEIGHT_OF(pPos2)>_UI2_) pPos2 = pPos2->prev;
        if(!pPos2) return _FALSE;

        pPos3 = pPos2->prev;

        if( pPos2->code!=_GU_ && pPos2->code!=_GUs_) return _FALSE;

        h = HEIGHT_OF(pPos2);

        if(h != _UE2_ && h != _UI1_ && h != _UI2_) return _FALSE;
        if(pPos3==_NULL || pPos3->prev==_NULL) goto OK;
        if( x[MID_POINT(pPos2)] < x[MID_POINT(pPos3)] ) return _FALSE;

        goto OK;
    }
    return _FALSE;
    OK:
    return _TRUE;
}

/*****************************************************************************/
/***** FOR ENGLISH ***********************************************************/
/*****************************************************************************/

#define APS_SIZE 6

_BOOL IsNearI( p_SPECL );

_BOOL RestoreApostrophEnglish( p_low_type low_data, p_SPECL pCurr )
{
    _INT lang = low_data->rc->lang;
    _SHORT bAfter;
    _RECT rc;
    p_SHORT x = low_data->x;
    p_SHORT y = low_data->y;
    p_SHORT xBuf = low_data->xBuf;
    //  p_SHORT yBuf  = low_data->yBuf;
    p_SHORT iBack = low_data->buffers[2].ptr;

    _INT dxAbs, dyAbs, xPos, yPos;
    _LONG dlAbs;
    p_SPECL pPos;
    p_SPECL next;
    p_SPECL prev;
    _INT iSafe;

    _BOOL bPit = _FALSE;
    _BOOL bTop = _FALSE;

    _BOOL bInv = _FALSE; /* apostrophe bend:  \ (not | or /) */
    _BOOL bLng = _FALSE;
    _BOOL bDxt = _FALSE;
    _BOOL bNextUp;
    _BOOL bPrevUp;
    _BOOL bLftFar = _TRUE;
    _BOOL bRgtFar = _TRUE;
    _BOOL bBigBrk = _FALSE;

    //return(_FALSE);

    if ( pCurr->code != _ST_ )
        return (_FALSE);

    /* height */
    xPos = x[MID_POINT(pCurr)];
    yPos = y[MID_POINT(pCurr)];
    if ( yPos > STR_UP + ONE_THIRD(STR_SIZE) )
    {
        return (_FALSE);
    }
    /* look around: are up elements near? */
    yPos = HWRMax(y[pCurr->ibeg],y[pCurr->iend]);
    pPos = FindApsPlace(lang, low_data, pCurr, _FALSE, &bAfter );

    if ( pPos == _NULL )
    {
        return (_FALSE);
    }
    next = GetNextUpElem( pPos );
    prev = GetPrevUpElem( pPos );

    iSafe = -10;
    while ( next != _NULL && prev != _NULL && x[MID_POINT(prev)] > x[MID_POINT(next)] )
    {
        _INT dxPrev = x[MID_POINT(prev)] - xPos;
        _INT dxNext = x[MID_POINT(next)] - xPos;
        _INT dx;

        TO_ABS_VALUE(dxPrev);
        TO_ABS_VALUE(dxNext);

        if ( dxPrev < dxNext )
        {
            //next = prev;
            dx = x[MID_POINT(prev)] - xPos;
        }
        else
        {
            //prev = next;
            dx = x[MID_POINT(next)] - xPos;
        }
        if ( dx < 0 )
            next = GetNextUpElem( next );
        else
            prev = GetPrevUpElem( prev->prev );

        if ( ++iSafe == 0 )
        {
            break;
        }
    }
    if ( next == _NULL || prev == _NULL ) // Up to Guitman...
    {
        return (_FALSE);
    }

    /*    if(IsAnyBreak(pPos) || IsAnyBreak(pPos->next)) // Check: is it big break?
     {
     p_SPECL pBrk;
     _SHORT  dl,dx=-1;
     _SHORT  n  = 0;
     _SHORT  av = 0;

     for(pBrk=low_data->specl->next;pBrk!=_NULL;pBrk=pBrk->next)
     {
     if(!IsAnyBreak(pBrk)) continue;

     if(y[pBrk->ibeg]<0)   continue;
     if(y[pBrk->iend]<0)   continue;
     if(y[pBrk->ibeg] >= y[pBrk->iend]) continue;

     if(pBrk == pPos || pBrk == pPos->next)
     {
     dx = x[pBrk->iend] - x[pBrk->ibeg];
     continue;
     }
     n++;
     dl = x[pBrk->iend] - x[pBrk->ibeg];

     av+=dl;
     }
     if(n>0)
     {
     av = ONE_NTH(av,n);
     if(dx > MULT_RATIO(av,3,2)) bBigBrk=_TRUE;
     }
     }
     */
    /* Block: Is far? */
    {
        _INT i;
        _INT dl, pos;

        GetTraceBox( x, y, pCurr->ibeg, pCurr->iend, (p_RECT) &rc );
        dl = STR_SIZE / 16 + (rc.right - rc.left) / 2;
        pos = (rc.left + rc.right) / 2; //rc.left+(rc.right-rc.left)/2;

        for ( i = prev->ibeg; i < next->iend; i++ )
        {
            if ( x[i] < pos - dl || x[i] > pos + dl )
                continue;
            if ( i >= pCurr->ibeg && i <= pCurr->iend )
                continue;

            if ( x[i] <= pos )
                bLftFar = _FALSE;
            else
                bRgtFar = _FALSE;
        }
    }
    /* Block */
    {
        _SHORT lPos, rPos;

        lPos = (_SHORT) HWRMin(next->ibeg,next->iend);
        rPos = (_SHORT) HWRMax(next->ibeg,next->iend);
        GetTraceBox( x, y, lPos, rPos, (p_RECT) &rc );
        bNextUp = yPos > rc.top;

        lPos = (_SHORT) HWRMin(prev->ibeg,prev->iend);
        rPos = (_SHORT) HWRMax(prev->ibeg,prev->iend);
        GetTraceBox( x, y, lPos, rPos, (p_RECT) &rc );
        if ( prev->prev != _NULL && prev->prev->prev != _NULL )
        {
            p_SPECL XT1 = prev->prev;
            p_SPECL XT2 = prev->prev->prev;
            p_SPECL XT3 = prev->prev->prev->prev;

            bDxt = (XT1->code == _XT_ && XT2->code == _XT_) && (XT3 == _NULL || XT3->code != _XT_) && (HEIGHT_OF(XT1) <= _UI1_
                    || HEIGHT_OF(XT1) >= _DI2_) && (HEIGHT_OF(XT2) <= _UI1_ || HEIGHT_OF(XT2) >= _DI2_);

        }
        bPrevUp = yPos > rc.top || bDxt;

        bTop = !bNextUp && !bPrevUp && !bLftFar && !bRgtFar;
        bPit = bNextUp && bPrevUp;
    }
    // bTop, bPit **********
    if ( bTop )
    {
        return (_FALSE);
    }
    if ( !bPit && bAfter )
    {
        return (_FALSE);
    }
    // Lenght:
    dxAbs = x[pCurr->ibeg] - x[pCurr->iend];
    dyAbs = y[pCurr->ibeg] - y[pCurr->iend];
    TO_ABS_VALUE(dxAbs);
    TO_ABS_VALUE(dyAbs);
    dlAbs = dxAbs * dxAbs + dyAbs * dyAbs;

    bLng = (dlAbs > (ONE_NTH(STR_DOWN-yPos,4) * ONE_NTH(STR_DOWN-yPos,4)));

    /* BEND: \|/ */
    if ( y[pCurr->ibeg] < y[pCurr->iend] )
    {
        bInv = (xBuf[iBack[pCurr->ibeg]] < xBuf[iBack[pCurr->iend]]);
    }
    else
    {
        bInv = (xBuf[iBack[pCurr->ibeg]] > xBuf[iBack[pCurr->iend]]);
    }

    /* compare apostrophe lenght with line height */
    if ( bPit || (bLftFar && bRgtFar) )
    {
        if ( dlAbs < (ONE_NTH(STR_DOWN-yPos,16) * ONE_NTH(STR_DOWN-yPos,16)) )
        {
            return (_FALSE);
        }
    }
    else
    {
        if ( dlAbs < (ONE_NTH(STR_DOWN-yPos, bDxt?10:6) * ONE_NTH(STR_DOWN-yPos, bDxt?10:6)) )
        {
            return (_FALSE);
        }
    }
    /* Block: BEND */
    {
        //_SHORT dxAbs = xBuf[iBack[pCurr->ibeg]]-xBuf[iBack[pCurr->iend]];
        //_SHORT dyAbs = yBuf[iBack[pCurr->ibeg]]-yBuf[iBack[pCurr->iend]];

        //TO_ABS_VALUE(dxAbs);
        //TO_ABS_VALUE(dyAbs);

        if ( bPit || bLng || (bLftFar && bRgtFar) )
        {
            if ( MULT_RATIO(dxAbs,10,30) > dyAbs )
            {
                return (_FALSE);
            }
        }
        else
        {
            if ( MULT_RATIO(dxAbs,10,15) > dyAbs )
            {
                return (_FALSE);
            }
        }
        if ( (bInv || !bPit) && !bLftFar )
        {
            if ( MULT_RATIO(dxAbs,11,10) > dyAbs )
            {
                return (_FALSE);
            }
        }
    }
    if ( !bPit && ((!bPrevUp && IsNearI( prev )) || (!bNextUp && IsNearI( next ))) )
    {
        _BOOL bFlg = _TRUE;
        p_SPECL pPnt;
        _SHORT xPnt;

        if ( !bPrevUp && IsNearI( prev ) && prev->prev != _NULL && prev->prev->code == _XT_ )
        {
            bFlg = _FALSE;
        }
        else if ( !bNextUp && IsNearI( next ) && next->prev->code == _XT_ )
        {
            bFlg = _FALSE;
        }
        if ( !bPrevUp && IsNearI( prev ) && bLftFar && bBigBrk )
        {
            bFlg = _FALSE;
        }
        if ( !bNextUp && IsNearI( next ) && bRgtFar && bBigBrk )
        {
            bFlg = _FALSE;
        }
        else
        {
            for ( pPnt = low_data->specl->next; pPnt != _NULL; pPnt = pPnt->next )
            {
                int x0, x1;

                if ( pPnt->code != _ST_ )
                    continue;
                if ( pCurr == pPnt )
                    continue;

                if ( !bPrevUp && IsNearI( prev ) )
                {
                    x0 = x[prev->ibeg];
                    x1 = xPos;
                    x0 -= xPos - x0;
                }
                else
                {
                    x1 = x[next->ibeg];
                    x0 = xPos;
                    x1 += x1 - xPos;
                }
                xPnt = x[MID_POINT(pPnt)];

                if ( xPnt > x0 && xPnt < x1 )
                    bFlg = _FALSE;
            }
        }
        if ( bFlg )
        {
            return (_FALSE);
        }
    }
    /* Block: Is covered? */
    {
        _INT i;

        GetTraceBox( x, y, pCurr->ibeg, pCurr->iend, (p_RECT) &rc );
        rc.left -= (_SHORT) 2;
        rc.right += (_SHORT) 2;

        for ( i = prev->ibeg; i < next->iend; i++ )
        {
            if ( x[i] < rc.left || x[i] > rc.right )
                continue;
            if ( i >= pCurr->ibeg && i <= pCurr->iend )
                continue;

            if ( y[i] < rc.bottom )
            {
                return (_FALSE);
            }
        }
    }
    /* OK! It's apostrophe! */
    /* Move to nearest element: */
    FindApsPlace(lang, low_data, pCurr, _TRUE, &bAfter );

    return (_TRUE);
}

/************************************************/

/*  Returns _TRUE, if the place was found; _FALSE otherwise. */

p_SPECL FindApsPlaceEnglish( low_type _PTR low_data, p_SPECL pPoint, _BOOL bMove, p_SHORT bAfter )
{
    p_SHORT x = low_data->x;
    p_SHORT y = low_data->y;
    p_SPECL pCur;
    p_SPECL pBest;
    _INT dxBest, dxCur, xCur, xColon;

    _BOOL bFind;
    _BOOL bShft = _FALSE;
    _BOOL bCurr;
    p_SPECL pNewBrk;

    /*  Find the best place to put apostroph: */

    xColon = x[MID_POINT(pPoint)];

    /* Scan upper elements to find the best apostroph place:  */

    for ( pCur = low_data->specl->next, dxBest = ALEF, pBest = _NULL; pCur != _NULL; pCur = pCur->next )
    {

        if ( pCur != pPoint && IsUpperSpl( pCur ) )
        {
            if ( pCur->prev == _NULL )
            {
                if ( pCur->next == _NULL || y[pCur->iend] == BREAK )
                {
                    return _NULL;
                }
                xCur = x[pCur->iend];
            }
            else if ( pCur->next == _NULL )
            {
                if ( y[pCur->ibeg] == BREAK )
                {
                    return _NULL;
                }
                xCur = x[pCur->ibeg];
            }
            else
            {
                if ( y[pCur->ibeg] == BREAK || y[pCur->iend] == BREAK )
                {
                    return _NULL;
                }
                if ( IsAnyBreak( pCur ) )
                {
                    if ( x[pCur->ibeg] < x[pCur->iend] )
                        xCur = x[MID_POINT(pCur)];
                    else
                        xCur = x[pCur->ibeg];
                }
                else
                    xCur = x[MID_POINT(pCur)];
            }
            dxCur = xCur - xColon;
            bCurr = dxCur > 0;
            TO_ABS_VALUE(dxCur);

            if ( dxCur < dxBest )
            {
                pBest = pCur;
                dxBest = dxCur;
                bShft = bCurr;
            }
        }
    }
    /*  If there is no place to put apostroph, just exit: */
    if ( pBest == _NULL )
        return _NULL;

    if ( bShft && pBest->prev != _NULL )
        pBest = pBest->prev;

    bFind = _FALSE;
    *bAfter = _TRUE;
    while ( pBest )
    {
        if ( IsUpperSpl( pBest ) )
        {
            if ( x[MID_POINT(pBest)] < xColon )
                break;
        }
        if ( pBest->prev != _NULL )
            pBest = pBest->prev;
        if ( pBest == low_data->specl || IsAnyBreak( pBest ) )
        {
            bFind = _TRUE;
            *bAfter = _FALSE;
            break;
        }
    }
    if ( !bFind )
        while ( pBest->next )
        {
            if ( IsUpperSpl( pBest->next ) )
            {
                if ( x[MID_POINT(pBest->next)] > xColon )
                    break;
            }
            if ( IsAnyBreak( pBest->next ) )
            {
                *bAfter = _FALSE;
                break;
            }
            pBest = pBest->next;
        }

    if ( !bMove )
        return (pBest);

    if ( !IsAnyBreak( pBest ) )
    {
        pNewBrk = NewSPECLElem( low_data );
        if ( pNewBrk == _NULL )
            return _NULL;
        Insert2ndAfter1st( pBest, pNewBrk );
        pNewBrk->ibeg = pNewBrk->iend = pBest->iend;
        pBest = pNewBrk;
    }
    pBest->code = _FF_;
    pBest->mark = DROP;
    pBest->other = NO_PENALTY;
    ASSIGN_HEIGHT(pBest,_MD_);

    AdjustBegEnd( pPoint );

    Move2ndAfter1st( pBest, pPoint );
    if ( pBest->ibeg < pPoint->ibeg )
        pBest->iend = pPoint->ibeg;

    if ( pPoint->next != _NULL )
    {
        if ( !IsAnyBreak( pPoint->next ) )
        {
            pNewBrk = NewSPECLElem( low_data );
            if ( pNewBrk == _NULL )
                return _FALSE;
            Insert2ndAfter1st( pPoint, pNewBrk );
        }
        pNewBrk = pPoint->next;

        pNewBrk->code = _FF_;
        pNewBrk->mark = DROP;
        pNewBrk->other = NO_PENALTY;
        ASSIGN_HEIGHT(pNewBrk,_MD_);

        pNewBrk->ibeg = pNewBrk->iend = pPoint->iend;
    }
    return pBest;
}

/************************************************/

_BOOL IsNearI( p_SPECL pPos )
{
    if ( !pPos )
        return (_FALSE);

    if ( pPos->code == _IU_ )
    {
        if ( pPos->mark != MINW && pPos->mark != BEG && pPos->mark != STICK )
        {
            return (_FALSE);
        }
    }
    else if ( pPos->code == _UUL_ )
    {
        if ( !pPos->prev )
            return (_FALSE);
        if ( pPos->prev->code != _Z_ )
            return (_FALSE);
    }
    else
        return (_FALSE);

    pPos = SkipAnglesAfter( pPos );

    if ( !pPos )
        return (_FALSE);

    if ( pPos->code != _ID_ && pPos->code != _UDR_ )
        return (_FALSE);

    if ( pPos->mark != END )
        return (_FALSE);

    return (_TRUE);
}

_BOOL RestoreApostroph(_INT lang, p_low_type low_data,p_SPECL pCurr)
{
    switch(lang) {
    case LANGUAGE_FRENCH:
    case LANGUAGE_ITALIAN:
    case LANGUAGE_PORTUGUESE:
        return RestoreApostrophFrench(low_data, pCurr);
    default:
        return RestoreApostrophEnglish(low_data, pCurr);
    }
}

p_SPECL FindApsPlace(_INT lang, low_type _PTR low_data,
    p_SPECL pPoint, _BOOL bMove, p_SHORT bAfter )
{
    switch (lang)
    {
    case LANGUAGE_FRENCH:
    case LANGUAGE_ITALIAN:
    case LANGUAGE_PORTUGUESE:
        return FindApsPlaceFrench(low_data, pPoint, bMove, bAfter);
    default:
        return FindApsPlaceEnglish(low_data, pPoint, bMove, bAfter);
    }
}

#endif //#ifndef LSTRIP
